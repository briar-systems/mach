# mach.lang.me.pipeline

## def OptLevel

```mach
pub def OptLevel: u8
```

## val OPT_DEBUG

```mach
pub val OPT_DEBUG:   OptLevel = 0
```

## val OPT_RELEASE

```mach
pub val OPT_RELEASE: OptLevel = 1
```

## rec VectorizeEnabledOption

```mach
pub rec VectorizeEnabledOption;
```

## rec SimdRequireOption

```mach
pub rec SimdRequireOption;
```

## rec FloatReassocOption

```mach
pub rec FloatReassocOption;
```

## rec PipelineRequest

```mach
pub rec PipelineRequest;
```

diags is the module's diagnostic store: a shape or `simd = "require"` refusal
is a located error there, never a bare failure

## fun run

```mach
pub fun run(req: *PipelineRequest) res[bool, fail.Fail];
```

