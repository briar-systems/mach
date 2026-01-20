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
  - [Output format](#output-format)
- [Configuration](#configuration)
- [How it works](#how-it-works)


## Overview

The test framework allows you to define tests directly in your `.mach` source files alongside your implementation code.
When you run `cmach test`, the compiler:

1. Scans all `.mach` files in the project's source directory
2. Collects all `test` blocks from each file
3. Transforms them into executable test functions
4. Builds and runs each module's tests as a separate binary
5. Reports results per-module and in aggregate


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
cmach test [--target <name>] [path]
```

| Option             | Description                                           |
|--------------------|-------------------------------------------------------|
| `--target <name>`  | Select a specific target from `mach.toml`             |
| `path`             | Project directory (defaults to current directory)     |

Examples:

```sh
# run all tests in current project
cmach test .

# run tests for a specific target
cmach test --target linux .
```


### Output format

The test runner outputs results per-module, then a final summary:

```
[test] mach.tests.example (3)
pass: example: first test
pass: example: second test
fail: example: broken test
[fail] mach.tests.example (1 failed)
[test] mach.tests.other (2)
pass: other: test one
pass: other: test two
[pass] mach.tests.other
tests: 5, passed: 4, failed: 1
```

Each line shows:
- `pass:` or `fail:` prefix indicating the test result
- The test name as defined in the source

Module-level status:
- `[pass]` — all tests in the module passed
- `[fail]` — one or more tests failed (shows count)
- `[crash]` — the test process terminated due to a signal (e.g., segfault)

The final summary shows total counts:
- `tests` — total number of tests across all modules
- `passed` — tests that returned non-zero
- `failed` — tests that returned zero
- `crashed` — modules that terminated via signal (only shown if > 0)
- `compile errors` — modules that failed to compile (only shown if > 0)


## Configuration

Tests require the `dir_tests` field in your target configuration.
This specifies where test binaries are written:

```toml
[project]
id = "myapp"
dir_src = "src"
dir_out = "out"

[targets.linux]
os = "linux"
isa = "x86_64"
abi = "sysv64"
dir_tests = "tests"
# ...
```

With this configuration, test binaries are placed under:

```
out/linux/tests/
```

See [config.md](config.md#dir_tests) for more details.


## How it works

The test harness performs these steps for each module containing tests:

1. **Parse** the source file and identify all `test` blocks
2. **Transform** each test block into a named function that returns `i64`
3. **Generate** a `__mach_test_main` function that:
   - Calls each test function
   - Checks if the return value is non-zero (pass) or zero (fail)
   - Prints the result with the test name
   - Tracks total failures
4. **Generate** a `_start` entrypoint that calls `__mach_test_main` and exits with the failure count
5. **Compile** the transformed module to an object file
6. **Link** the object file into a standalone executable (using `cc -nostdlib`)
7. **Execute** the binary and capture the exit code
8. **Report** results based on the exit code:
   - Exit code 0 means all tests passed
   - Exit code 1-128 indicates the number of failed tests
   - Exit code > 128 indicates the process was killed by a signal

Each module's tests run in isolation as a separate process.
This ensures that a crash in one module doesn't prevent other modules from being tested.