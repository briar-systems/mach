# mach.lang.target.of.spv

## rec SpvSummary

```mach
pub rec SpvSummary;
```

## fun validate

```mach
pub fun validate(buf: *u8, len: usize) res[SpvSummary, fail.Fail];
```

## val DEBUG

```mach
pub val DEBUG: target_of.DebugVTable = target_of.DebugVTable;
```

the spirv model is written into the module by the emitter: names, sources and line markers

## val VTABLE

```mach
pub val VTABLE: target_of.OfVTable = target_of.OfVTable;
```

