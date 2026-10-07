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

## rec SimdRequireOption

```mach
pub rec SimdRequireOption;
```

## rec PassSet

```mach
pub rec PassSet;
```

a set of the schedule's passes, one bit per row of `PASSES`

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

## fun pass_count

```mach
pub fun pass_count() usize;
```

how many passes the schedule runs

## fun pass_at

```mach
pub fun pass_at(i: usize) *pass.Pass;
```

pass `i` of the schedule, in the order `PASSES` lists them

## fun pass_named

```mach
pub fun pass_named(name: str) opt[usize];
```

the index of the pass named `name`, none when the schedule runs no such pass

## fun pass_level

```mach
pub fun pass_level(i: usize) OptLevel;
```

the lowest level any row runs pass `i` at: OPT_DEBUG for a pass every build
runs unless it is skipped, OPT_RELEASE for one `optimize` selects

## fun set_empty

```mach
pub fun set_empty() PassSet;
```

## fun set_has

```mach
pub fun set_has(s: PassSet, i: usize) bool;
```

## fun set_add

```mach
pub fun set_add(s: *PassSet, i: usize);
```

## fun set_remove

```mach
pub fun set_remove(s: *PassSet, i: usize);
```

