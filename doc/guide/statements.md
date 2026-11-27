# Statements

Control flow is `if` with `or`, one loop called `for`, and `fin` for cleanup. There is no `else`, `while`, `switch` or `defer`.

## `if` and `or`

```mach
fun sign(n: i64) i64 {
    if (n < 0) {
        ret -1;
    }
    or (n == 0) {
        ret 0;
    }
    or {
        ret 1;
    }
}
```

`or (cond)` adds an arm, and a bare `or` is the fallback. Every body is braced. A condition is a `u1`, so an integer is compared: `if (n != 0)`, never `if (n)`.

## `for`

```mach
fun first_even(xs: *i64, n: u64) i64 {
    var i: u64 = 0;
    for (i < n) {
        val x: i64 = xs[i];
        i = i + 1;
        if (x % 2 != 0) { cnt; }
        ret x;
    }
    ret -1;
}
```

| Form | Meaning |
|---|---|
| `for (cond) { }` | Loop while `cond` holds |
| `for { }` | Loop until a `brk` or a `ret` |
| `brk;` | Leave the innermost `for` |
| `cnt;` | Start its next pass |

There is no counting form and no for-each. The counter is a `var` that the body steps.

## `ret`

`ret expr;` returns a value, and `ret;` leaves a function that has no result. A function with a result ends every path in `ret`, as [Functions](functions.md) describes.

## `fin`

```mach
use std.print;

fun work() {
    fin { print.println("last"); }
    fin { print.println("first"); }
    print.println("working");
}
```

`fin { }` runs its block when the enclosing block exits, whether by reaching its end or by `ret`, `brk` or `cnt`.

- Several run latest first.
- A `fin` covers only the statements after it.
- In a loop body it runs at the end of every pass.
- The value of a `ret` is computed before the fins run.
- A `fin` body cannot `ret`.

## Blocks

```mach
fun scoped() i64 {
    var n: i64 = 1;
    {
        val tmp: i64 = n * 2;
        n = tmp + 1;
    }
    ret n;
}
```

A bare `{ }` is a scope of its own. Its locals end at the closing brace, and its fins run there.

## Missing keywords

| Elsewhere | In Mach |
|---|---|
| `else`, `else if` | `or`, `or (cond)` |
| `while`, `loop` | `for` |
| `switch`, `match` | An `if` and `or` chain |
| `defer` | `fin { }` |

The parser reads these words as plain names, so one of them gives a syntax error about what follows it.

The full rules are in the reference for [statements](../language/statements.md).
