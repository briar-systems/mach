# mach.lang.be.codegen.regalloc

register allocation for a MIR module: every function is coalesced, given
its scan order, scanned in rounds until no spilled value earns a piece or a
rematerialization, then rewritten onto physical registers and verified.
each phase is a module of its own beside this one, and they share one
regalloc.context.Context per function

## fun run

```mach
pub fun run(tgt: *lang_target.Binding, m: *lang_mir.MirModule) err[fail.Fail];
```

