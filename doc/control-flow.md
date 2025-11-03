# Control Flow

This document describes Mach’s runtime control flow constructs: conditional `if`/`or` chains, loops with `for`, loop control with `brk` and `cnt`, returns with `ret`, and blocks `{ ... }`.

Related topics:
- [Lexical Structure](lexical-structure.md)
- [Expressions and Operators](expressions-and-operators.md)
- [Variables and Constants](variables.md)
- For compile-time conditions, see [Compile-time Features](compile-time.md)

Mach uses `u8` as the boolean type for conditions: zero is false; nonzero is true.

- Statements terminated with `;`: `ret`, `brk`, `cnt`, expression statements, and declaration statements (`use`, `ext`, `def`, `val`, `var`).
- Statements not terminated with `;`: `if`/`or` blocks and `for` blocks (they end with `}`).

## Blocks and scope

A block is a sequence of statements enclosed by braces:

```
{
    # statements
}
```

- Blocks introduce a new scope. Declarations inside a block are not visible outside it.
- Nested blocks are allowed.
- Blocks appear after `if`, `or`, and `for`, and may be used anywhere a statement is allowed.

See [Variables and Constants](variables.md) for scoping rules.

## Conditionals: if/or chains

Mach uses `if` with optional `or` branches to form conditional chains.

Basic forms:
```
if (condition) {
    # then-block
}

if (cond1) {
    # then-block
}
or (cond2) {
    # else-if block
}
or {
    # else block
}
```

Notes:
- The condition expression is parenthesized: `if (expr)` and `or (expr)`. The `or` branch condition is optional; an `or` without a condition is the final “else” branch.
- Multiple `or` branches can follow an `if` (and can chain from another `or`).
- An `or` branch must immediately follow the preceding `if` or `or` block; it cannot start a new chain on its own.
- Conditions are evaluated as `u8` (zero/false, nonzero/true). See [Expressions and Operators](expressions-and-operators.md).

Example:
```
fun classify(n: u64) u64 {
    if (n == 0) {
        ret 0;
    }
    or (n < 10) {
        ret 1;
    }
    or {
        ret 2;
    }
}
```

## Loops: for with optional condition

`for` introduces a loop with an optional parenthesized condition:

- Infinite loop:
  ```
  for {
      # body
  }
  ```

- While-style loop:
  ```
  for (condition) {
      # body
  }
  ```

Behavior:
- In `for { ... }`, the body executes repeatedly without a loop condition.
- In `for (cond) { ... }`, the condition is evaluated before each iteration; loop continues while the condition is nonzero (`u8` truth).
- Use ordinary statements in the body to update state and eventually terminate the loop (via `brk`, by making the condition false, or by `ret`).

Typical patterns:
```
# counting loop
var i: u64 = 0;
for (i < 10) {
    # work
    i = i + 1;
}

# processing until break
for {
    if (should_stop()) {
        brk;
    }
    step();
}
```

## Loop control: brk and cnt

Within a loop body:
- `brk;` breaks out of the innermost enclosing loop.
- `cnt;` continues to the next iteration of the innermost loop (skips the remainder of the current iteration).

Examples:
```
# break on match
for (i < n) {
    if (arr[i] == target) {
        brk;
    }
    i = i + 1;
}

# skip even numbers
var x: u64 = 0;
for (x < 20) {
    if ((x % 2) == 0) {
        x = x + 1;
        cnt;
    }
    process(x);
    x = x + 1;
}
```

Rules:
- `brk;` and `cnt;` are statements and must end with `;`.
- They affect only the innermost `for` loop.

## Returns: ret

`ret` exits the current function.

Forms:
- `ret;` — return from a function with no return value.
- `ret expression;` — return a value.

Examples:
```
fun done() {
    ret;
}

fun add(a: u64, b: u64) u64 {
    ret a + b;
}
```

Rules:
- In a function declared without a return type, use `ret;` without an expression.
- In a function with a declared return type, the returned expression must be type‑compatible with that type. Use an explicit cast `::` when necessary. See [Expressions and Operators](expressions-and-operators.md).

## Putting it together

```
fun find_first(hay: []u8, needle: u8) i64 {
    var i: u64 = 0;

    if (hay.len == 0) {
        ret -1;
    }
    or (hay.len == 1) {
        ret (hay[0] == needle) :: i64;
    }

    for (i < hay.len) {
        if (hay[i] == needle) {
            ret i :: i64;
        }
        i = i + 1;
    }

    ret -1;
}
```

This function:
- Uses an `if`/`or` chain for fast paths.
- Iterates with a `for (i < hay.len)` loop.
- Returns early when a match is found.
- Ends with a final `ret` when no match exists.

For expression syntax, precedence, and casting, see [Expressions and Operators](expressions-and-operators.md). For compile-time conditionals (e.g., `$if (...) { ... } or { ... }`), see [Compile-time Features](compile-time.md).