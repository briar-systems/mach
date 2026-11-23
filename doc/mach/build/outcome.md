[mach](../README.md) / [build](README.md) / **outcome**

# mach.build.outcome

what a build produced: artifacts, test dispatchers, declassifications, worst severity

an outcome owns everything it records, and a refused record leaves it as it was. a
failure is a `fail.Fail` and folds into the severity, while the failures and
diagnostics themselves went to the build's readout sink as they happened.

Source: [`src/build/outcome.mach`](../../../src/build/outcome.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.report.testrun`](../report/testrun.md).

| Type | | |
| :--- | :--- | :--- |
| [`BuildSeverity`](#buildseverity) | `def` |  |
| [`BuildOutcome`](#buildoutcome) | `rec` | what a build produced |
| [`Declassified`](#declassified) | `rec` | one declassification of the source, a `:>T` |

| On `BuildSeverity` | |
| :--- | :--- |
| [`BUILD_OK`](#buildseverity), [`BUILD_USER`](#buildseverity), [`BUILD_ENVIRONMENT`](#buildseverity), [`BUILD_INTERNAL`](#buildseverity) | |
| [`severity_of`](#severity_of) | the build severity a failure stands at |

| On `BuildOutcome` | |
| :--- | :--- |
| [`outcome_init`](#outcome_init) | an empty outcome whose records `oa` will own |
| [`outcome_dnit`](#outcome_dnit) | release an outcome and everything it recorded; nil is a no-op |
| [`record_artifact`](#record_artifact) | every record_* copies what it stores into the outcome's allocator; a refused copy or push leaves the outcome exactly as it was and releases the copies made before the refusal |
| [`record_test`](#record_test) | t's text is borrowed; the outcome keeps its own copy |
| [`record_declassified`](#record_declassified) | d's texts are borrowed; the outcome keeps its own copies |
| [`fold_fail`](#fold_fail) | a reported or user failure is the user's; the severity never regresses |
| [`merge`](#merge) | capacity is reserved for all three transfers before any element moves, so a refused reservation leaves both outcomes exactly as they were |

---

## BuildSeverity

```mach
pub def BuildSeverity: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `BUILD_OK` | `0` |  |
| `BUILD_USER` | `1` |  |
| `BUILD_ENVIRONMENT` | `2` |  |
| `BUILD_INTERNAL` | `3` |  |

### severity_of

```mach
pub fun severity_of(f: *mach.base.fail.Fail) BuildSeverity
```

the build severity a failure stands at

| Parameter | |
| :--- | :--- |
| `f` | the failure |
| **returns** | BUILD_ENVIRONMENT for an environment failure, BUILD_INTERNAL for an internal message, BUILD_USER otherwise |

## BuildOutcome

```mach
pub rec BuildOutcome {
    severity:     BuildSeverity;
    artifacts:    Vector[Artifact];
    tests:        Vector[mach.report.testrun.TestArtifact];
    declassified: Vector[Declassified];
}
```

what a build produced. owned by the allocator outcome_init was given: every
artifact path and test artifact in it is released together by outcome_dnit, and
a refused record_* leaves the outcome exactly as it was. what happened along the
way, failures and diagnostics included, went to the build's readout sink

| Field | Type | |
| :--- | :--- | :--- |
| `severity` | `BuildSeverity` | the worst standing recorded, BUILD_OK to BUILD_INTERNAL; ordered, compared with > |
| `artifacts` | `Vector[Artifact]` | the outputs written, in build order |
| `tests` | `Vector[mach.report.testrun.TestArtifact]` | the test dispatchers built, for the runner |
| `declassified` | `Vector[Declassified]` | the declassifications a check listed, in module then source order |

### outcome_init

```mach
pub fun outcome_init(oa: *std.allocator.Allocator) BuildOutcome
```

an empty outcome whose records `oa` will own

### outcome_dnit

```mach
pub fun outcome_dnit(bo: *BuildOutcome)
```

release an outcome and everything it recorded; nil is a no-op

### record_artifact

```mach
pub fun record_artifact(bo: *BuildOutcome, kind: Spelled, path: str) err[std.allocator.Error]
```

every record_* copies what it stores into the outcome's allocator; a refused
copy or push leaves the outcome exactly as it was and releases the copies
made before the refusal

### record_test

```mach
pub fun record_test(bo: *BuildOutcome, t: mach.report.testrun.TestArtifact) err[std.allocator.Error]
```

t's text is borrowed; the outcome keeps its own copy. a nil text stays nil

### record_declassified

```mach
pub fun record_declassified(bo: *BuildOutcome, d: Declassified) err[std.allocator.Error]
```

d's texts are borrowed; the outcome keeps its own copies

### fold_fail

```mach
pub fun fold_fail(bo: *BuildOutcome, f: *mach.base.fail.Fail)
```

a reported or user failure is the user's; the severity never regresses

### merge

```mach
pub fun merge(agg: *BuildOutcome, unit: *BuildOutcome) err[std.allocator.Error]
```

capacity is reserved for all three transfers before any element moves, so
a refused reservation leaves both outcomes exactly as they were

## Declassified

```mach
pub rec Declassified {
    file:     str;
    line:     usize;
    column:   usize;
    function: str;
    secret:   str;
}
```

one declassification of the source, a `:>T`

| Field | Type | |
| :--- | :--- | :--- |
| `file` | `str` | the path of its file |
| `line` | `usize` | its line, from 1 |
| `column` | `usize` | its column, from 1 |
| `function` | `str` | the function or `test <name>` it stands in, "\<module>" outside both |
| `secret` | `str` | the type of the value it declassifies, as the source spells it |

