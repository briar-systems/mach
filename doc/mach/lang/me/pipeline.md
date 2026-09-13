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

## fun run

```mach
pub fun run(req: *PipelineRequest) res[bool, fail.Fail];
```

