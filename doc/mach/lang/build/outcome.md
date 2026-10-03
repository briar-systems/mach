# mach.lang.build.outcome

## rec TestArtifact

```mach
pub rec TestArtifact;
```

a collected test: its qualified name, where it is declared, the test object
that holds it, the dispatcher that runs it as `<exe> <idx>`, and the target and
profile that dispatcher was built for

## def BuildSeverity

```mach
pub def BuildSeverity: u8
```

## val BUILD_OK

```mach
pub val BUILD_OK:          BuildSeverity = 0
```

## val BUILD_USER

```mach
pub val BUILD_USER:        BuildSeverity = 1
```

## val BUILD_ENVIRONMENT

```mach
pub val BUILD_ENVIRONMENT: BuildSeverity = 2
```

## val BUILD_INTERNAL

```mach
pub val BUILD_INTERNAL:    BuildSeverity = 3
```

## rec BuildOutcome

```mach
pub rec BuildOutcome;
```

what a build produced. owned by the allocator outcome_init was given: every
artifact path and test artifact in it is released together by outcome_dnit, and
a refused record_* leaves the outcome exactly as it was. what happened along the
way, failures and diagnostics included, went to the build's readout sink

severity: the worst standing recorded, BUILD_OK to BUILD_INTERNAL; ordered, compared with >
artifacts: the outputs written, in build order
tests: the test dispatchers built, for the runner

## fun outcome_init

```mach
pub fun outcome_init(oa: *A.Allocator) BuildOutcome;
```

an empty outcome whose records `oa` will own

## fun outcome_dnit

```mach
pub fun outcome_dnit(bo: *BuildOutcome);
```

release an outcome and everything it recorded; nil is a no-op

## fun record_artifact

```mach
pub fun record_artifact(bo: *BuildOutcome, kind: catalog_artifact.Kind, path: str) err[A.Error];
```

every record_* copies what it stores into the outcome's allocator; a refused
copy or push leaves the outcome exactly as it was and releases the copies
made before the refusal

## fun record_test

```mach
pub fun record_test(bo: *BuildOutcome, t: TestArtifact) err[A.Error];
```

t's text is borrowed; the outcome keeps its own copy. a nil text stays nil

## fun severity_of

```mach
pub fun severity_of(f: *fail.Fail) BuildSeverity;
```

## fun fold_fail

```mach
pub fun fold_fail(bo: *BuildOutcome, f: *fail.Fail);
```

a reported or user failure is the user's; the severity never regresses

## fun merge

```mach
pub fun merge(agg: *BuildOutcome, unit: *BuildOutcome) err[A.Error];
```

capacity is reserved for both transfers before any element moves, so
a refused reservation leaves both outcomes exactly as they were

## rec GateTally

```mach
pub rec GateTally;
```

## fun gate_tally_init

```mach
pub fun gate_tally_init(a: *A.Allocator) GateTally;
```

## fun record_gate

```mach
pub fun record_gate(gt: *GateTally, a: *A.Allocator, r: validation.ValidationGateResult) err[A.Error];
```

