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

Both are operand-less, so they are keywords only in their bare `brk;` / `cnt;`
form. The same word followed by anything else is an ordinary identifier — a
variable named `cnt` reads and assigns normally (`cnt = x;`), even inside a
loop that also uses bare `cnt;` for control flow.

## `fin` — deferred block

`fin` schedules a block to execute when its enclosing block exits, in reverse
order of declaration. Useful for cleanup that should happen regardless of how
the scope exits.

```mach
{
    fin { counter = counter - 1; }
    fin { counter = counter * 2; }

    # ... code ...
}
# at block exit, in reverse order: counter * 2 runs first, then counter - 1
```

`fin` is block-scoped: it belongs to the block that declares it and covers
only the statements after its declaration. A function body is a block, so the
classic pattern — a `fin` at the top of a function running at every return —
is the block rule applied to the outermost scope.

- A **normal block exit** replays the block's fins, then execution continues
  after the block.
- A **loop body** is a block: its fins replay at the end of every iteration,
  with that iteration's values.
- **`brk` / `cnt`** replay the fins of every scope being exited — everything
  down through the targeted loop's body, innermost first — before branching.
  The loop body's fins replay under `cnt` too: the iteration is ending.
- **`ret expr;`** fully evaluates the return expression first, then replays
  the fins of every open scope (innermost first), then returns the
  already-evaluated value — a fin's side effects are never observable in the
  returned value. A bare `ret` likewise, minus the expression.

A `fin` declared inside another fin's body belongs to that body's block and
replays when the body finishes. Control flow cannot cross a `fin` boundary
outward: a `ret` inside a fin body is a compile error, as is a `brk` / `cnt`
whose target loop encloses the fin. A loop fully inside the fin body uses
`brk` / `cnt` normally.

`fin` requires a block body (`fin { ... }`). The bare single-statement form
(`fin stmt;`) is rejected.

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
