# mach.lang.me.transform.tailcall

tail calls (#3417)

a call is in tail position when its block returns the call's result, or
returns nothing after a call that returns nothing, with only debug
annotations between the call and the return. no `fin` can be pending there,
since a fin's code runs between the two. a function takes part only when
nothing of its frame can outlive it into the callee: it is not naked or
oblivious, it runs no inline asm, and no stack allocation's address
escapes. an aggregate a call takes by value does not escape, since the
convention hands the callee a copy.

`recurse` turns a self call in tail position into a branch back to a loop
header whose phis carry the parameters. `ret f(args) op x` with op an
integer add, multiply, or or xor is the same loop with an accumulator phi
holding the pending operands, and every other return returns the
accumulator combined with its value. it runs before inlining, so recursion
peeling sees the loop, and the loop is what licm and the vectorizer read.

`mark` flags every call left in tail position. it runs last in the release
pipeline, so no pass moves anything between a flagged call and its return,
and the backend makes a flagged call a jump where the calling convention
lets the callee take over the caller's frame.

a debug annotation of a call's result is dropped with the call, so the
variable reads as optimized out there; -g annotates and never decides

## fun recurse_in

```mach
pub fun recurse_in(m: *me_ir.Module, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

## fun mark_in

```mach
pub fun mark_in(m: *me_ir.Module, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

