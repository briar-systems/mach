# mach.lang.build.outcome

## tag Fail

```mach
pub tag Fail: u8 {
    reported;
    user:        Keyed;
    internal:    str;
    environment: Keyed;
}
```

the driver's failure: already reported through diagnostics, the input was
wrong (a manifest error, a missing project), an invariant the compiler owns
was broken, or the machine rather than the input (a tool that could not be
spawned, a resource that was not there). the exit code derives from the
case: 1 for user, 2 for internal, 3 where a command distinguishes the
environment; a compiler failure never renders as a test failure. `reported`
is declared first so a zero outcome is a failure that invents no text. a
user or environment failure names the diagnostic kind it is reported as; an
internal one is always `compiler.internal`

## rec Keyed

```mach
pub rec Keyed;
```

kind: the row of the diagnostic kind table the failure is reported as
text: the message
at:   where in the file that caused it the failure points, the zero place
      when it points nowhere
also: the other places the failure names, none when it names no other

## rec Related

```mach
pub rec Related;
```

the other places a failure names, in order, each in its own file: the other
edges of a cycle, the other claims a collision is between. the array and
each resolved place's path share the failure's lifetime as `at`'s path does

items: the places, nil when there are none
count: how many

## rec Place

```mach
pub rec Place;
```

where a failure points: the bytes [start, end) of the file at `path`, and the
line and column of `start` and of `end`, each from 1 with columns counted in
UTF-8 bytes. a place whose range is known before its file is `spanned` with a
nil path until `placed` names the file. the path shares the message's
lifetime: a copy of the failure that outlives its message copies both

path: the file, nil until the place is resolved
spanned: whether a range is known; false with a nil path is no place
start: the byte offset of the first byte
end: the byte offset just past the last byte
line: the line of `start`
col: the column of `start`
end_line: the line of `end`
end_col: the column of `end`

## fun reported

```mach
pub fun reported() Fail;
```

## fun user

```mach
pub fun user(k: diagnostic_kind.Kind, message: str) Fail;
```

a failure names a live row of the registry: one that names none, or a
retired one, is a compiler defect and becomes an internal failure

## fun internal

```mach
pub fun internal(message: str) Fail;
```

## fun spanned

```mach
pub fun spanned(f: Fail, start: usize, end: usize) Fail;
```

the same failure pointing at the bytes [start, end) of the file that caused
it, which `placed` names. a failure that already points somewhere keeps its
place, the innermost site knowing best, and one with no kind points nowhere

## fun placed

```mach
pub fun placed(a: *A.Allocator, f: Fail, path: str, text: str) Fail;
```

the same failure with its ranges resolved in `text`, the file at `path`, as
`located` places them: its own and each related one still without a file. a
failure without a range, or one already resolved, is returned as it is

## fun placed_bytes

```mach
pub fun placed_bytes(a: *A.Allocator, f: Fail, path: str, data: *u8, n: usize) Fail;
```

`placed` over the `n` bytes at `data`, for a file that may hold a NUL

## fun at

```mach
pub fun at(a: *A.Allocator, f: Fail, p: Place) Fail;
```

the same failure pointing at `p`, a place a model recorded: one without a
file is a range `placed` resolves, one with a file is copied as `located`
copies it. a failure that already points somewhere keeps its place

## fun with_related

```mach
pub fun with_related(a: *A.Allocator, f: Fail, ps: *Place, n: usize) Fail;
```

the same failure naming the `n` places at `ps` as its related places, in
order, each copied through `a` as `at` copies a place; an unspanned place is
skipped. a failure that already names related places keeps them, and one
whose copy is refused is that refusal

## fun related_of

```mach
pub fun related_of(f: Fail) Related;
```

the related places a failure names, none for one that names no other

## fun located

```mach
pub fun located(a: *A.Allocator, f: Fail, at: Place) Fail;
```

the same failure pointing at `at`, its path copied through `a` so the failure
owns it as it owns its text. a failure with no kind points nowhere, and one
whose copy is refused is that refusal

## fun place

```mach
pub fun place(path: str, data: *u8, n: usize, start: usize, end: usize) Place;
```

the bytes [start, end) of the `n` bytes at `data`, the file at `path`, as a
place; a range past the end is clamped to it

## rec Lines

```mach
pub rec Lines;
```

the offsets the lines of one file start at, for resolving many places in it
as `place` resolves one, each in time logarithmic in the file's lines

path: the file, which every place resolved here names
n: the file's length in bytes
starts: the offset of each line's first byte, the first line's 0

## fun lines_of

```mach
pub fun lines_of(a: *A.Allocator, path: str, data: *u8, n: usize) res[Lines, A.Error];
```

the lines of the `n` bytes at `data`, the file at `path`; released with `lines_dnit`

## fun lines_dnit

```mach
pub fun lines_dnit(l: *Lines);
```

## fun lines_place

```mach
pub fun lines_place(l: *Lines, start: usize, end: usize) Place;
```

the bytes [start, end) of the file `l` indexes as a place, as `place` makes it

## fun place_of

```mach
pub fun place_of(f: Fail) opt[Place];
```

where the failure points, when it points at a file

## fun environment

```mach
pub fun environment(k: diagnostic_kind.Kind, message: str) Fail;
```

## fun kind_of

```mach
pub fun kind_of(f: Fail) diagnostic_kind.Kind;
```

the kind the failure is reported as, NONE for a reported one, whose
diagnostics carry their own

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
pub fun write_text(e: io_writer.WriteError) str;
```

## fun fs_text

```mach
pub fun fs_text(e: fs.FsError) str;
```

## fun str_text

```mach
pub fun str_text(e: types_string.StrError) str;
```

## fun format_text

```mach
pub fun format_text(e: std_format.FormatError) str;
```

## fun toml_text

```mach
pub fun toml_text(e: toml.TomlError) str;
```

## fun toml_failure

```mach
pub fun toml_failure(e: toml.TomlError) Fail;
```

a document that does not parse is the user's, pointing at the byte the parser
refused; one the allocator refused is internal

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

## fun with_text

```mach
pub fun with_text(f: Fail, message: str) Fail;
```

the same failure carrying `message` instead: the case and the place are
kept, the text replaced (a caller that copies the message into storage it owns)

## fun retain

```mach
pub fun retain(a: *A.Allocator, f: Fail) res[Fail, A.Error];
```

a copy of the failure that owns its text and its places through `a`, for a
failure that outlives the storage its message was made in; released with
`release`

## fun release

```mach
pub fun release(a: *A.Allocator, f: Fail);
```

release what a retained failure owns through `a`: its text and its places

## fun retain_places

```mach
pub fun retain_places(a: *A.Allocator, f: Fail) res[Fail, A.Error];
```

a copy of the failure whose places, its own path and every related place,
are owned through `a`, its text left as it is; for a holder that keeps the
text apart. released with `release_places`

## fun release_places

```mach
pub fun release_places(a: *A.Allocator, f: Fail);
```

release the places a failure owns through `a`, as `retain_places` made them

## fun without_places

```mach
pub fun without_places(f: Fail) Fail;
```

the same failure pointing nowhere and naming no related place, for a holder
whose places were released or never copied

## fun same_places

```mach
pub fun same_places(x: Fail, y: Fail) bool;
```

whether two failures hold the same places: the same path storage and the
same related array, as a holder that copied them once sees its own copy

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

a language failure keeps its class and its kind

## fun to_fail

```mach
pub fun to_fail(f: Fail) fail.Fail;
```

a driver failure met inside a language pass (an output write): the class
and the kind are kept

## fun user_fail

```mach
pub fun user_fail(k: diagnostic_kind.Kind, f: fail.Fail) Fail;
```

a phase failure the user caused (target selection, source loading, import
libraries): an unkeyed text becomes the user class under `k`, a keyed
failure keeps its own class and kind, a reported failure stays reported

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
that holds it, the dispatcher that runs it as `<exe> <idx>`, and the target and
profile that dispatcher was built for

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
    fail:        FailEvent;
    diagnostics: DiagnosticBatch;
}
```

what a build recorded, in order: a unit finished, a failure, or a batch of
diagnostics with the sources they refer to. every payload is owned by the
outcome's allocator

## rec FailEvent

```mach
pub rec FailEvent;
```

a failure recorded outside a diagnostic store, and the phase it is reported under

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
pub fun record_fail(bo: *BuildOutcome, f: *Fail, origin: diagnostic.Origin) err[A.Error];
```

the failure is copied whole: its text into the outcome's allocator

origin: the phase the failure is reported under

## fun record_diagnostics

```mach
pub fun record_diagnostics(bo: *BuildOutcome, sources: *lang_source.SourceMap, diags: *diagnostic.DiagnosticStore) err[Fail];
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

