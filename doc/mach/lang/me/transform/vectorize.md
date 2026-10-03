# mach.lang.me.transform.vectorize

## fun run

```mach
pub fun run(m: *me_ir.Module, tgt: *resolved.Target, float_reassoc: bool, itn: *intern.Interner) res[bool, fail.Fail];
```

`itn` names the constant globals a splat of a constant reads from; nil
keeps every splat on the stack

