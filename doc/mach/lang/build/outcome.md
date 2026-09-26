# mach.lang.build.outcome

## tag Fail

```mach
pub tag Fail: u8 {
    reported;
    user:        str;
    internal:    str;
    environment: str;
}
```

the driver's failure: already reported through diagnostics, the input was
wrong (a manifest error, a missing project), an invariant the compiler owns
was broken, or the machine rather than the input (a tool that could not be
spawned, a resource that was not there). the exit code derives from the
case: 1 for user, 2 for internal, 3 where a command distinguishes the
environment; a compiler failure never renders as a test failure. `reported`
is declared first so a zero outcome is a failure that invents no text

## fun reported

```mach
pub fun reported() Fail;
```

## fun user

```mach
pub fun user(message: str) Fail;
```

## fun internal

```mach
pub fun internal(message: str) Fail;
```

## fun environment

```mach
pub fun environment(message: str) Fail;
```

## fun refused

```mach
pub fun refused(e: A.Error) Fail;
```

an allocation refusal stays an internal failure (exit 2), as it always has;
reclassifying it as environmental changes exit codes and is the CLI lane's

## fun alloc_text

```mach
pub fun alloc_text(e: A.Error) str;
```

the text a std refusal renders as when a driver operation reports it: the
operation keeps its own classification (user, internal, environment) and
names the cause once through these. `alloc.text` is the allocator's

## fun io_text

```mach
pub fun io_text(e: io_error.Error) str;
```

## fun read_text

```mach
pub fun read_text(e: reader.ReadError) str;
```

## fun write_text

```mach
pub fun write_text(e: writer.WriteError) str;
```

## fun fs_text

```mach
pub fun fs_text(e: fs.FsError) str;
```

## fun str_text

```mach
pub fun str_text(e: string.StrError) str;
```

## fun format_text

```mach
pub fun format_text(e: format.FormatError) str;
```

## fun toml_text

```mach
pub fun toml_text(e: toml.TomlError) str;
```

## fun env_text

```mach
pub fun env_text(e: env.EnvError) str;
```

## fun unit

```mach
pub fun unit[T](r: res[T, Fail]) err[Fail];
```

the unit outcome of an operation whose value is not needed

## fun is_reported

```mach
pub fun is_reported(f: Fail) bool;
```

## fun is_internal

```mach
pub fun is_internal(f: Fail) bool;
```

## fun text

```mach
pub fun text(f: Fail) opt[str];
```

the message a failure carries, absent for a reported one

## fun describe

```mach
pub fun describe(f: Fail) str;
```

the failure as one line of presentation text; a reported failure has no
text of its own and is named as such, never as an empty message

## fun release_text

```mach
pub fun release_text(a: *A.Allocator, f: Fail);
```

release the text a failure owns through `a`; a reported failure owns nothing

## fun with_text

```mach
pub fun with_text(f: Fail, message: str) Fail;
```

the same failure carrying `message` instead: the case is kept, the text
replaced (a caller that copies the message into storage it owns)

## fun catalog

```mach
pub fun catalog(a: *A.Allocator, c: fail.Catalog) Fail;
```

the closed-catalog policy lifted into the driver's kind: an input or
capability fault is the user's, a compiler-produced member is internal. the
message belongs to the caller's allocator

## fun unknown_catalog

```mach
pub fun unknown_catalog(a: *A.Allocator, catalog_name: str, tag: u32) Fail;
```

## fun from_fail

```mach
pub fun from_fail(f: fail.Fail) Fail;
```

## fun user_fail

```mach
pub fun user_fail(f: fail.Fail) Fail;
```

a phase failure the user caused (target selection, source loading, import
libraries): its text becomes the user class, a reported failure stays reported

## def ArtifactKind

```mach
pub def ArtifactKind: u8
```

## val ART_BINARY

```mach
pub val ART_BINARY:  ArtifactKind = 0
```

## val ART_OBJECTS

```mach
pub val ART_OBJECTS: ArtifactKind = 1
```

## val ART_ARCHIVE

```mach
pub val ART_ARCHIVE: ArtifactKind = 2
```

## val ART_SHARED

```mach
pub val ART_SHARED:  ArtifactKind = 3
```

## val ART_TESTS

```mach
pub val ART_TESTS:   ArtifactKind = 4
```

## rec Artifact

```mach
pub rec Artifact;
```

## rec TestArtifact

```mach
pub rec TestArtifact;
```

a collected test: its qualified name, where it is declared, the test object
that holds it, and the dispatcher that runs it as `<exe> <idx>`

## rec BuildUnitEvent

```mach
pub rec BuildUnitEvent;
```

verb: what the unit's goal calls working on it, borrowed from the goal catalog
rather than owned, since every spelling there is a static string

## rec DiagnosticBatch

```mach
pub rec DiagnosticBatch;
```

## tag BuildEvent

```mach
pub tag BuildEvent: u8 {
    unit:        BuildUnitEvent;
    fail:        Fail;
    diagnostics: DiagnosticBatch;
}
```

what a build recorded, in order: a unit finished, a failure, or a batch of
diagnostics with the sources they refer to. every payload is owned by the
outcome's allocator

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

what a build produced and recorded. owned by the allocator outcome_init was given:
every artifact path, test artifact, event payload and failure text in it is released
together by outcome_dnit, and a refused record_* leaves the outcome exactly as it was

severity: the worst standing recorded, BUILD_OK to BUILD_INTERNAL; ordered, compared with >
artifacts: the outputs written, in build order
tests: the test dispatchers built, for the runner
events: everything recorded, in order

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
pub fun record_artifact(bo: *BuildOutcome, kind: ArtifactKind, path: str) err[A.Error];
```

every record_* copies what it stores into the outcome's allocator; a refused
copy or push leaves the outcome exactly as it was and releases the copies
made before the refusal

## fun record_test

```mach
pub fun record_test(bo: *BuildOutcome, t: TestArtifact) err[A.Error];
```

t's text is borrowed; the outcome keeps its own copy. a nil text stays nil

## fun record_unit

```mach
pub fun record_unit(bo: *BuildOutcome, artifact: str, target: str, verb: str, has_artifact: bool) err[A.Error];
```

## fun record_fail

```mach
pub fun record_fail(bo: *BuildOutcome, f: *Fail) err[A.Error];
```

the failure is copied whole: its text into the outcome's allocator

## fun record_diagnostics

```mach
pub fun record_diagnostics(bo: *BuildOutcome, sources: *source.SourceMap, diags: *diagnostic.DiagnosticStore) err[Fail];
```

the batch is a composite of two subsystems' snapshots, so its refusal is
the driver's failure: an internal one carrying the snapshot's own text, or
the allocation refusal of the push

## fun severity_of

```mach
pub fun severity_of(f: *Fail) BuildSeverity;
```

## fun fold_fail

```mach
pub fun fold_fail(bo: *BuildOutcome, f: *Fail);
```

a reported or user failure is the user's; the severity never regresses

## fun merge

```mach
pub fun merge(agg: *BuildOutcome, unit: *BuildOutcome) err[A.Error];
```

capacity is reserved for all three transfers before any element moves, so
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

