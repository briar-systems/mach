# Testing

Mach includes a built-in test framework for writing and running tests within your project.
Tests are defined using the `test` keyword and executed via the `cmach test` command.

- [Overview](#overview)
- [Writing tests](#writing-tests)
  - [Test syntax](#test-syntax)
  - [Pass and fail semantics](#pass-and-fail-semantics)
  - [Using helper functions](#using-helper-functions)
- [Running tests](#running-tests)
  - [Command syntax](#command-syntax)
  - [Filtering tests](#filtering-tests)
  - [Output format](#output-format)
- [Architecture](#architecture)
  - [Per-test isolation](#per-test-isolation)
  - [Crash detection](#crash-detection)
  - [Platform portability](#platform-portability)
- [How it works](#how-it-works)


## Overview

The test framework allows you to define tests directly in your `.mach` source files alongside your implementation code.
When you run `cmach test`, the compiler:

1. Scans all `.mach` files in the project's source directory
2. Collects all `test` blocks from each file
3. Compiles each test as a separate executable
4. Runs each test individually in its own process
5. Reports results per-test and in aggregate


## Writing tests

### Test syntax

Tests are defined at the top level of a module using the `test` keyword:

```mach
test "descriptive name" {
    # test body
}
```

The test name is a string literal that describes what the test verifies.
The body executes in a function-like context with access to all symbols in the module.


### Pass and fail semantics

A test passes if it returns a non-zero value and fails if it returns zero:

```mach
test "addition works" {
    var result: i64 = 2 + 2;
    if (result != 4) { ret 0; }  # fail
    ret 1;  # pass
}
```

If a test body completes without an explicit `ret`, the harness automatically appends `ret 1` (pass).

A common pattern for assertions:

```mach
test "multiple checks" {
    if (some_function() != expected_value) { ret 0; }
    if (another_function() != other_value) { ret 0; }
    ret 1;
}
```


### Using helper functions

Tests can call any function defined in the same module or imported from other modules:

```mach
fun factorial(n: i64) i64 {
    if (n <= 1) { ret 1; }
    ret n * factorial(n - 1);
}

test "factorial of 5" {
    if (factorial(5) != 120) { ret 0; }
    ret 1;
}

test "factorial of 0" {
    if (factorial(0) != 1) { ret 0; }
    ret 1;
}
```


## Running tests

### Command syntax

```
cmach test [options] [path]
```

| Option               | Description                                           |
|----------------------|-------------------------------------------------------|
| `--target <name>`    | Select a specific target from `mach.toml`             |
| `--filter <pattern>` | Only run tests whose name or module path matches      |
| `-m`, `--modules`    | Show module-level progress during execution           |
| `-h`, `--help`       | Show help message                                     |
| `path`               | Project directory (defaults to current directory)     |

Examples:

```sh
# run all tests in current project
cmach test .

# run tests for a specific target
cmach test --target linux .

# run only tests matching a pattern
cmach test --filter "lexer" .

# show module progress
cmach test -m .
```


### Filtering tests

The `--filter` option matches against both module paths and test names.
A test runs if the pattern appears anywhere in either:

```sh
# runs tests in modules containing "parser"
cmach test --filter parser .

# runs tests with "edge case" in their name
cmach test --filter "edge case" .
```


### Output format

By default, the test runner shows only failures.
Passing tests are silent unless you use verbose options.

For each failing test:

```
fail: module_name: test name here
```

For crashed tests:

```
crash: module_name: test name here
```

The final summary shows aggregate counts:

```
--- results ---
passed:  145
failed:  3
crashed: 1
total:   149
```

With `-m` (modules), you also see progress per module:

```
[test] mach.tests.lexer (15)
[test] mach.tests.parser (23)
fail: parser: unexpected token handling
[test] mach.tests.sema (8)
```


## Architecture

### Per-test isolation

Each test runs in its own process.
The test runner compiles each test block into a separate executable, then executes them one at a time.

Benefits:
- A crash in one test doesn't prevent other tests from running
- Test output is naturally isolated
- Side effects from one test can't affect others
- Exact identification of which test crashed

The previous architecture bundled all tests in a module into a single executable.
If any test crashed (e.g., null pointer dereference), the entire module failed and remaining tests were skipped.
The new architecture eliminates this problem.


### Crash detection

When a test process terminates abnormally (signal on Unix, exception on Windows), the test runner detects it and reports a crash:

| Exit code   | Meaning                                |
|-------------|----------------------------------------|
| 0           | Test passed (returned non-zero)        |
| 1           | Test failed (returned zero)            |
| > 128       | Test crashed (signal number = code-128)|

On Unix, `SIGSEGV` (11) results in exit code 139.
The runner continues to the next test after any crash.


### Platform portability

Test executables use `std.runtime` for their entry point.
This module provides a portable `_start` that:
1. Calls the test's `main` function
2. Exits with the return code

Because `std.runtime` handles platform-specific details, tests work across all supported targets without inline assembly in the test harness.

Adding support for a new platform requires only updating `std.runtime` — the test runner itself requires no changes.


## How it works

The test harness performs these steps:

1. **Scan** all `.mach` files in the project's source directory
2. **Parse** each file and identify `test` blocks
3. **For each test:**
   a. Clone the original AST, excluding other tests
   b. Add `use std.runtime;` for portable entry point
   c. Transform the test body into a named function returning `i64`
   d. Generate a `main` function that:
      - Calls the test function
      - Returns 0 if test passed (non-zero return)
      - Returns 1 if test failed (zero return)
   e. Compile to a standalone executable
   f. Execute the binary
   g. Record the result based on exit code
4. **Report** aggregate results

Test binaries are placed in a hidden `.tests` directory under the output path:

```
out/<target>/.tests/<module>/<test_name>
```

For example, a test named `"lexer: simple identifier"` in module `mach.compiler.lexer` might produce:

```
out/linux/.tests/mach_compiler_lexer/lexer__simple_identifier
```

The test runner cleans up old test binaries before each run.