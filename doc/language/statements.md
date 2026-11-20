# Statements

Statement forms compose into function bodies and blocks. Statements end
with `;` except where they end with a block `{...}`.

## `if` / `or`

```mach fragment
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
- Bodies are blocks. There is no one-statement-without-braces form.

A condition has type `u1`, which comparisons and logical operators produce. Any
other type is refused with `condition.type`, naming `u1`. A v6 condition of type
`u8` is refused the same way, naming the comparison that replaces it.

An `if`/`or` arm whose condition is exactly `sel P.c` guards the payload place
`P.c` inside its block:

```mach
tag Reply: u8 { empty; value: i64; }

fun read(reply: Reply) i64 {
    if (sel reply.value) {
        ret reply.value; # guarded by the arm condition
    }
    or {
        ret 0; # reply holds empty on this path
    }
}
```

A guard is a lexical region, not a flow fact. A chain whose every arm exits
guards the remainder of the enclosing block for the case the chain left
untested, and a chain's final bare `or` is guarded the same way for the one case
the arms before it left untested. Assigning a name the guarded place is spelled
with, such as the `i` of `sel items[i].value`, ends the guard from that point on,
for the whole of a loop that holds the assignment and in a `fin` body that
replays after it. See [tag.md](tag.md).

## `for`

A single condition-loop form. There is no for-each.

```mach
use std.print;
use std.runtime;

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

```mach
use std.print;
use std.runtime;

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

```mach fragment
ret expr;                   # return a value
ret;                        # return from a void function
```

## `brk` / `cnt`

Loop control: `brk` exits the enclosing `for`, and `cnt` continues to the
next iteration. A `brk` or `cnt` with no enclosing `for` in its function is
refused with `jump.no_loop`.

```mach
use std.print;
use std.runtime;

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
form. The same word followed by anything else is an ordinary identifier. A
variable named `cnt` reads and assigns normally (`cnt = x;`), even inside a
loop that also uses bare `cnt;` for control flow.

## `fin`: deferred block

`fin` schedules a block to execute when its enclosing block exits, in reverse
order of declaration. Useful for cleanup that should happen regardless of how
the scope exits.

```mach
use std.print;
use std.runtime;

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
classic pattern, a `fin` at the top of a function running at every return,
is the block rule applied to the outermost scope.

- A **normal block exit** replays the block's fins, then execution continues
  after the block.
- A **loop body** is a block: its fins replay at the end of every iteration,
  with that iteration's values.
- **`brk` / `cnt`** replay the fins of every scope being exited, everything
  down through the targeted loop's body, innermost first, before branching.
  The loop body's fins replay under `cnt` too: the iteration is ending.
- **`ret expr;`** fully evaluates the return expression first, then replays
  the fins of every open scope (innermost first), then returns the
  already-evaluated value. A fin's side effects are never observable in the
  returned value. A bare `ret` likewise, minus the expression.

A `fin` declared inside another fin's body belongs to that body's block and
replays when the body finishes. Control flow cannot cross a `fin` boundary
outward: a `ret` inside a fin body is refused with `fin.ret`, and a `brk` /
`cnt` whose target loop encloses the fin is refused with `fin.jump`. A loop
fully inside the fin body uses `brk` / `cnt` normally.

`fin` requires a block body (`fin { ... }`). The bare single-statement form
(`fin stmt;`) is rejected, and so is a `ret` inside a `fin` body:

```mach error `ret` cannot appear inside a `fin` body
fun leave() i64 {
    fin { ret 1; }
    ret 0;
}
```

## Block

`{ ... }` introduces a new lexical scope. Statements inside are evaluated
in order, and the storage of a local declared in it ends when control leaves it
(see [val-var.md](val-var.md#scope)). Blocks can stand alone:

```mach fragment
{
    val tmp: i64 = compute();
    use_tmp(tmp);
}
```

## Expression statements

An expression followed by a semicolon executes as a statement:

```mach fragment
compute();
```

Assignment is an expression (`x = y;` is an expression statement whose top
operator is `=`), and so is a call whose result is discarded.

An assignment gives the place the value the right side had before the
assignment, always. A record, array, tag or union is copied byte for byte, and
the storage on the two sides may overlap: two members of a `uni` at different
offsets assigned to each other, or two pointers into one buffer, copy as if the
whole right side were read before any of the left side is written. So, with
`p` an integer address such as a `usize` (a pointer takes no `+`),
`@(p::*[16]u64) = @((p + 8)::*[16]u64);` moves sixteen words down by one word,
and its mirror moves them up. The one limit on "always" is a pointer that
reaches a local in a way the language does not define
([operators.md](operators.md#where-locals-lie)).

## Failure handling

A function that can fail returns a tag. The caller tests the case with `sel`
and exits the arm that handles the failure. The exiting chain guards the
success payload for the rest of the block:

```mach
use std.types.error.err;
use std.types.result.res;

tag WriteError: u8 { closed; full; }

fun flush() err[WriteError] {
    ret err[WriteError].ok{};
}

fun parse(input: u8) res[i64, WriteError] {
    ret res[i64, WriteError].ok{input::i64};
}

fun increment(input: u8) res[i64, WriteError] {
    val err_flushed: err[WriteError]     = flush();
    if (sel err_flushed.err) { ret res[i64, WriteError].err{err_flushed.err}; }
    val res_parsed: res[i64, WriteError] = parse(input);
    if (sel res_parsed.err) { ret res[i64, WriteError].err{res_parsed.err}; }
    ret res[i64, WriteError].ok{res_parsed.ok + 1}; # res_parsed.ok is guarded: the chain above exits
}
```

Every reachable path through the failure arm must leave the block, with `ret`,
or with `brk` or `cnt` targeting a loop that encloses the chain. An arm that
falls through opens no guard, and the payload read after it is rejected with
`tag.payload_unguarded`. See
[tag.md](tag.md) for the guard rules.

## See also

- [expressions.md](expressions.md) - expressions and literals
- [tag.md](tag.md) - tagged values, `sel` and guards
- [comptime-control.md](comptime-control.md) - the comptime counterpart
