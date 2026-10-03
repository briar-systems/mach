# mach.lang.be.codegen.frame

## fun run

```mach
pub fun run(tgt: *binding.Binding, m: *lang_mir.MirModule) err[fail.Fail];
```

## fun prune

```mach
pub fun prune(tgt: *binding.Binding, m: *lang_mir.MirModule) err[fail.Fail];
```

before selection, a frame slot address no instruction reads is dropped and a
slot nothing names any more leaves the frame, so a dead address costs
neither an instruction nor stack. an address or slot a debug binding
or an asm block names stays, since the debugger or the block reads it

