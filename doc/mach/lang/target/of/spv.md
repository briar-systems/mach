# mach.lang.target.of.spv

## rec SpvSummary

```mach
pub rec SpvSummary;
```

## fun validate

```mach
pub fun validate(buf: *u8, len: usize) res[SpvSummary, fail.Fail];
```

## fun debug_model

```mach
pub fun debug_model() of.DebugVTable;
```

the spirv model is written into the module by the emitter: names, sources and line markers

## fun register

```mach
pub fun register(reg: *of.OfRegistry) err[fail.Fail];
```

