# Statements

Statement forms compose into function bodies and blocks. Statements end
with `;` except where they end with a block `{...}`.

## `if` / `or`

```mach
if (cond) {
    ...
} or (cond) {
    ...
} or {
    ...                     # final else (no condition)
}
```

- The `if` head opens the chain.
- Each `or (cond) { ... }` adds another branch.
- A trailing `or { ... }` is the catch-all.
- Bodies are blocks; there is no one-statement-without-braces form.

## `for`

A single condition-loop form. There is no for-each.

```mach
var i: i64 = 0;
for (i < 10) {
    i = i + 1;
}
```

## `ret`

```mach
ret expr;                   # return a value
ret;                        # return from a void function
```

## `brk` / `cnt`

Loop control: `brk` exits the enclosing `for`; `cnt` continues to the
next iteration.

```mach
for (i < 10) {
    if (i == 3) { cnt; }
    if (i == 8) { brk; }
    i = i + 1;
}
```

## `fin` — deferred statement

`fin` schedules a statement (or block) to execute when the enclosing scope
exits, in reverse order of declaration. Useful for cleanup that should
happen regardless of how the scope exits.

```mach
{
    fin counter = counter - 1;
    fin { counter = counter * 2; }

    # ... code ...
}
# at scope exit: fin block runs first, then fin counter = counter - 1
```

`fin` takes either a single statement (`fin stmt;`) or a block
(`fin { ... }`). No bare-expression form.

## Block

`{ ... }` introduces a new lexical scope. Statements inside are evaluated
in order. Blocks can stand alone:

```mach
{
    val tmp: i64 = compute();
    use_tmp(tmp);
}
```

## See also

- [expressions.md](expressions.md) — what goes into the right side of `=`
  etc.
- [comptime-control.md](comptime-control.md) — the comptime counterpart
