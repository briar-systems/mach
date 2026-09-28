# mach.lang.me.transform.thread

jump threading over a boolean phi

a block that holds nothing but a phi of a condition and a conditional branch
on it is the join the lowering emits for `a && b` and `a || b`: each
predecessor arrives knowing the outcome, either as a constant or as the
compare it just computed, and then re-tests it. every predecessor that
reaches the join with an unconditional branch is redirected to the branch's
target directly, or branches on its own compare, and the join keeps only the
predecessors that could not be threaded.

## fun run_in

```mach
pub fun run_in(m: *ir.Module, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

