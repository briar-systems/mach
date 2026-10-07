# mach.lang.me.opt.mem2reg

promotion of memory to registers. a scalar local whose address is only
loaded from and stored to becomes ssa values: a phi where its stores meet,
at the dominance frontiers of the blocks that store it, and each load the
value that reaches it. unreachable blocks are pruned first, so every block
has a dominator. an aggregate, a local whose address escapes and one an
inline asm binds stay in memory. the debug variable on a promoted local
follows its values, a phi taking a binding of its own

## fun run

```mach
pub fun run(m: *me_ir.Module) res[bool, fail.Fail];
```

a run that owns its workspace

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

