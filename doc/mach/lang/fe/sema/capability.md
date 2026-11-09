# mach.lang.fe.sema.capability

the comptime capabilities a sema walk publishes for each of its phases

## fun semantic_name_caps

```mach
pub fun semantic_name_caps() res[comptime_capability.PhaseCapabilities[sema_context.SemaContext], fail.Fail];
```

## fun semantic_type_caps

```mach
pub fun semantic_type_caps() res[comptime_capability.PhaseCapabilities[sema_context.SemaContext], fail.Fail];
```

