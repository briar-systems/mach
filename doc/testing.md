# Testing

Mach includes a built-in test framework. Tests are defined using the `test` keyword and executed with `mach test`.


## Writing Tests

Tests are top-level declarations in any `.mach` file:

```mach
test "addition works" {
    var result: i64 = 2 + 2;
    if (result != 4) { ret 0; }
    ret 1;
}
```

The test name is a string literal. The body executes in a function-like context with access to all symbols in the module.


## Pass and Fail

- Return non-zero (e.g. `ret 1;`): test passes
- Return zero (`ret 0;`): test fails
- If no explicit `ret`, the harness appends `ret 1` (pass)

A common pattern:

```mach
test "multiple checks" {
    if (some_function() != expected) { ret 0; }
    if (another_function() != other) { ret 0; }
    ret 1;
}
```


## Using Imports and Helpers

Tests can call any function in the module or imported from other modules:

```mach
fun factorial(n: i64) i64 {
    if (n <= 1) { ret 1; }
    ret n * factorial(n - 1);
}

test "factorial of 5" {
    if (factorial(5) != 120) { ret 0; }
    ret 1;
}
```


## Running Tests

```
mach test [options] [path]
```

| Option | Description |
|--------|-------------|
| `--target <name>` | Select a specific target from `mach.toml` |
| `--filter <pattern>` | Only run tests matching the pattern |
| `-m`, `--modules` | Show module-level progress |
| `path` | Project directory (default: `.`) |

Examples:

```sh
mach test .
mach test --target linux .
mach test --filter "lexer" .
```


## Output

Passing tests are silent by default. Failures and crashes are reported:

```
fail: module_name: test name here
crash: module_name: test name here
```

The summary shows aggregate counts:

```
--- results ---
passed:  145
failed:  3
crashed: 1
total:   149
```

With `-m`, module progress is also shown:

```
[test] mach.tests.lexer (15)
[test] mach.tests.parser (23)
```


## Per-Test Isolation

Each test runs in its own process. A crash in one test (e.g. null pointer dereference) does not prevent other tests from running.

| Exit code | Meaning |
|-----------|---------|
| 0 | Passed (returned non-zero) |
| 1 | Failed (returned zero) |
| > 128 | Crashed (signal number = code - 128) |


## How It Works

1. Scan all `.mach` files in the project source directory
2. Parse each file and collect `test` declarations
3. For each test:
   - Transform the test body into a function returning `i64`
   - Generate a `main` function that calls it and maps the result to an exit code
   - Compile to a standalone executable
   - Execute and record the result
4. Report aggregate results

Test binaries are placed under the output directory in a `.tests` subdirectory.
