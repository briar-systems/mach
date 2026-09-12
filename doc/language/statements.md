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

An `if`/`or` arm whose condition is exactly `sel P.c` guards the payload place
`P.c` inside its block:

```mach accept
tag Reply: u8 { empty; value: i64; }

fun read(reply: Reply) i64 {
    if (sel reply.value) {
        ret reply.value;            # guarded by the arm condition
    }
    or {
        ret 0;                      # reply holds empty on this path
    }
}
```

A guard is a lexical region, not a flow fact. A chain whose every arm exits
guards the remainder of the enclosing block for the case the chain left
untested. See [tag.md](tag.md).

## `for`

A single condition-loop form. There is no for-each.

```mach run "10"
use std.runtime;
use print: std.print;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var i: i64 = 0;
    for (i < 10) {
        i = i + 1;
    }
    print.printlnf("{}", i);
    ret 0;
}
```

A `for` with no condition loops until a `brk` or a `ret` leaves it:

```mach run "3"
use std.runtime;
use print: std.print;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var i: i64 = 0;
    for {
        i = i + 1;
        if (i == 3) { brk; }
    }
    print.printlnf("{}", i);
    ret 0;
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

```mach run "1 2 4 5 6 7 8"
use std.runtime;
use print: std.print;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var i: i64 = 0;
    for (i < 10) {
        i = i + 1;
        if (i == 3) { cnt; }
        print.printf("{}", i);
        if (i == 8) { brk; }
        print.print(" ");
    }
    print.println("");
    ret 0;
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

```mach run "5 9"
use std.runtime;
use print: std.print;

var counter: i64 = 0;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    {
        fin { counter = counter - 1; }
        fin { counter = counter * 2; }

        counter = 5;
        print.printf("{} ", counter);
    }
    # at block exit, in reverse order: counter * 2 runs first, then counter - 1
    print.printlnf("{}", counter);
    ret 0;
}
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
(`fin stmt;`) is rejected, and so is a `ret` inside a `fin` body:

```mach reject "fin"
fun leave() i64 {
    fin { ret 1; }
    ret 0;
}
```

## Block

`{ ... }` introduces a new lexical scope. Statements inside are evaluated
in order. Blocks can stand alone:

```mach
{
    val tmp: i64 = compute();
    use_tmp(tmp);
}
```

## Expression statements

An expression followed by a semicolon executes as a statement:

```mach
compute();
```

Assignment is an expression (`x = y;` is an expression statement whose top
operator is `=`), and so is a call whose result is discarded.

## Failure handling

A function that can fail returns a tag. The caller tests the case with `sel`
and exits the arm that handles the failure; the exiting chain guards the
success payload for the rest of the block:

```mach accept
use std.types.canonical.res;
use std.types.canonical.err;

tag WriteError: u8 { closed; full; }

fun flush() err[WriteError] { ret err[WriteError].ok{}; }
fun parse(input: u8) res[i64, WriteError] { ret res[i64, WriteError].ok{input::i64}; }

fun increment(input: u8) res[i64, WriteError] {
    val flushed: err[WriteError] = flush();
    if (sel flushed.err) { ret res[i64, WriteError].err{flushed.err}; }
    val r: res[i64, WriteError] = parse(input);
    if (sel r.err) { ret res[i64, WriteError].err{r.err}; }
    ret res[i64, WriteError].ok{r.ok + 1};      # r.ok is guarded: the chain above exits
}
```

Every reachable path through the failure arm must leave the block, with `ret`,
or with `brk` or `cnt` targeting a loop that encloses the chain; an arm that
falls through opens no guard, and the payload read after it is rejected. See
[tag.md](tag.md) for the guard rules.

## See also

- [expressions.md](expressions.md) - expressions and literals
- [tag.md](tag.md) - tagged values, `sel` and guards
- [comptime-control.md](comptime-control.md) - the comptime counterpart
