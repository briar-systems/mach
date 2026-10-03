# mach.lang.fail

## tag Fail

```mach
pub tag Fail: u8 {
    reported;
    message:     str;
    user:        Keyed;
    environment: Keyed;
}
```

the compiler's failure, one type for every pass, the linker, the query
engine, the session and the build. the phase already recorded it as a
diagnostic and there is nothing further to say; an invariant the compiler
owns was broken, whose text must be preserved; the input was wrong or
unsupported; or the machine refused rather than the input (a file that could
not be read or written, a tool that could not be spawned). a user or
environment failure names the diagnostic kind it is reported as and may
point at a place; an internal one is always `compiler.internal`. the exit
code derives from the case: 1 for user, 2 for internal, 3 where a command
distinguishes the environment. `reported` is declared first so a zero
failure is one that invents no text. an allocation refusal is internal; the
compiler recovers from none of them at the site that met them

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

where a failure points: a span of the file at `path`, and the line and
column its first byte and the byte just past it fall on. a place whose span
is known before its file is `spanned` with a nil path until `placed` names
the file. the path shares the message's lifetime: a copy of the failure that
outlives its message copies both

path: the file, nil until the place is resolved
spanned: whether a span is known; false with a nil path is no place
span: the bytes the failure points at
start: the position of the span's first byte
end: the position just past the span's last byte

## fun reported

```mach
pub fun reported() Fail;
```

## fun message

```mach
pub fun message(text: str) Fail;
```

an internal failure: a compiler defect, reported as `compiler.internal`.
this is the one channel for a defect: a function that meets a broken
invariant returns it as a message failure, never as a placeholder, a default
answer or an abort. panic is left to a function documented as total, which
has no failure to return, when its caller breaks the precondition it states

## fun user

```mach
pub fun user(k: diagnostic_kind.Kind, text: str) Fail;
```

the input is wrong or unsupported. a failure names a live row of the
registry: one that names none, or a retired one, is a compiler defect and
becomes an internal failure

## fun environment

```mach
pub fun environment(k: diagnostic_kind.Kind, text: str) Fail;
```

the machine refused: a file could not be read or written, a tool could not
be spawned

## fun as_user

```mach
pub fun as_user(k: diagnostic_kind.Kind, f: Fail) Fail;
```

a failure the user caused met by a phase that cannot classify it (target
selection, source loading, import libraries): an internal text becomes the
user class under `k`, a keyed failure keeps its own class and kind, a
reported failure stays reported

## fun as_internal

```mach
pub fun as_internal(f: Fail) Fail;
```

a failure met on input the compiler produced itself (an object it wrote, a
module it emitted): whatever the check says of the input, it is the
compiler's defect, an internal failure keeping the text. a reported failure
stays reported

## fun spanned

```mach
pub fun spanned(f: Fail, s: lang_source.Span) Fail;
```

the same failure pointing at the span `s` of the file that caused it, which
`placed` names. a failure that already points somewhere keeps its place, the
innermost site knowing best, and one with no kind points nowhere

## fun span_only

```mach
pub fun span_only(s: lang_source.Span) Place;
```

a place whose span is known and whose file is not yet

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
pub fun place(path: str, lines: *lang_source.Lines, s: lang_source.Span) Place;
```

the span `s` of the file at `path` whose line index is `lines`, as a place;
a span past the end is clamped to it

## fun place_of

```mach
pub fun place_of(f: Fail) opt[Place];
```

where the failure points, when it points at a file

## fun kind_of

```mach
pub fun kind_of(f: Fail) diagnostic_kind.Kind;
```

the kind the failure is reported as, NONE for a reported one, whose
diagnostics carry their own

## fun is_reported

```mach
pub fun is_reported(f: Fail) bool;
```

## fun is_message

```mach
pub fun is_message(f: Fail) bool;
```

## fun text

```mach
pub fun text(f: Fail) opt[str];
```

the text a failure carries, absent for a reported one

## val REPORTED_TEXT

```mach
pub val REPORTED_TEXT: str = "failure was reported through diagnostics"
```

the failure as one line of presentation text; a reported failure has no
text of its own and is named as such, never as an empty message

## fun describe

```mach
pub fun describe(f: Fail) str;
```

## fun with_text

```mach
pub fun with_text(f: Fail, text: str) Fail;
```

the same failure carrying `text` instead: the class, the kind and the places
are kept (a caller that copies the message into storage it owns)

## fun retain

```mach
pub fun retain(a: *A.Allocator, f: Fail) res[Fail, A.Error];
```

a copy of the failure that owns its text and its places through `a`, for a
failure that outlives the storage its message was made in; released with
`dnit`

## fun dnit

```mach
pub fun dnit(a: *A.Allocator, f: Fail);
```

release what a retained failure owns through `a`: its text and its places

## fun places_retain

```mach
pub fun places_retain(a: *A.Allocator, f: Fail) res[Fail, A.Error];
```

a copy of the failure whose places, its own path and every related place,
are owned through `a`, its text left as it is; for a holder that keeps the
text apart. released with `places_dnit`

## fun places_dnit

```mach
pub fun places_dnit(a: *A.Allocator, f: Fail);
```

release the places a failure owns through `a`, as `places_retain` made them

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

## fun unit

```mach
pub fun unit[V](r: res[V, Fail]) err[Fail];
```

the unit outcome of an operation whose value is not needed

## fun io_text

```mach
pub fun io_text(e: io_error.Error) str;
```

the text each std refusal renders as, one copy each. an operation that meets
one keeps its own classification (user, internal, environment) and names the
cause once through these; the allocator's is `alloc.text`

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
pub fun str_text(e: StrError) str;
```

## fun format_text

```mach
pub fun format_text(e: std_format.FormatError) str;
```

## fun toml_text

```mach
pub fun toml_text(e: toml.TomlError) str;
```

## fun env_text

```mach
pub fun env_text(e: env.EnvError) str;
```

## fun format

```mach
pub fun format(a: *A.Allocator, fmt: str, va: ...) res[str, Fail];
```

`fmt` formatted with `va` into `a`, which owns the text; a refusal to format
is the failure instead

## fun formatted

```mach
pub fun formatted(a: *A.Allocator, fmt: str, va: ...) Fail;
```

an internal failure whose text is `fmt` formatted with `va` into `a`, which
owns the text; a refusal to format is the failure instead

## fun formatted_user

```mach
pub fun formatted_user(a: *A.Allocator, k: diagnostic_kind.Kind, fmt: str, va: ...) Fail;
```

a user failure under `k` whose text is `fmt` formatted with `va` into `a`,
which owns the text; a refusal to format is the failure instead

## fun message_of

```mach
pub fun message_of(made: res[str, Fail]) Fail;
```

the internal failure carrying a made text, or the failure that refused to
make it

## fun user_of

```mach
pub fun user_of(k: diagnostic_kind.Kind, made: res[str, Fail]) Fail;
```

the user failure under `k` carrying a made text, or the failure that refused
to make it

## fun text_retain

```mach
pub fun text_retain(itn: *intern.Interner, text: str) res[str, Fail];
```

the interner's copy of a borrowed `text`, which outlives the storage `text`
came from

## fun text_intern

```mach
pub fun text_intern(itn: *intern.Interner, a: *A.Allocator, text: str) res[str, Fail];
```

`text`, owned by `a`, moved into the interner: the interned copy outlives
`a`'s storage, and `text` is released either way

## fun format_intern

```mach
pub fun format_intern(itn: *intern.Interner, a: *A.Allocator, fmt: str, va: ...) res[str, Fail];
```

`fmt` formatted with `va` through `a` as scratch and interned, so the text
outlives `a`'s storage

## fun refused

```mach
pub fun refused(e: A.Error) Fail;
```

the refusals the compiler recovers from at no site, each an internal failure
carrying its text: the allocator's, a write to a sink of its own, a string
operation, a filesystem step and a format

## fun write_refused

```mach
pub fun write_refused(e: io_writer.WriteError) Fail;
```

## fun str_refused

```mach
pub fun str_refused(e: StrError) Fail;
```

## fun fs_refused

```mach
pub fun fs_refused(e: fs.FsError) Fail;
```

## fun format_refused

```mach
pub fun format_refused(e: std_format.FormatError) Fail;
```

## fun source_refused

```mach
pub fun source_refused(e: lang_source.Error) Fail;
```

## fun fs_environment

```mach
pub fun fs_environment(k: diagnostic_kind.Kind, e: fs.FsError) Fail;
```

a filesystem operation on a file the build was handed (an object, an
archive, a library) that the machine refused: the environment's under `k`,
save an allocation refusal, which stays internal

## fun toml_refused

```mach
pub fun toml_refused(e: toml.TomlError) Fail;
```

a document that does not parse is the user's, pointing at the byte the parser
refused; one the allocator refused is internal

## tag PhaseKind

```mach
pub tag PhaseKind: u8 {
    accepted;
    rejected;
    internal: str;
}
```

phase status is independent of diagnostic storage and rendering: accepted,
rejected through recorded diagnostics, or internally failed with the text
of the first internal failure

## rec PhaseStatus

```mach
pub rec PhaseStatus;
```

## fun accepted

```mach
pub fun accepted() PhaseStatus;
```

## fun reject

```mach
pub fun reject(s: *PhaseStatus);
```

## fun internal

```mach
pub fun internal(s: *PhaseStatus, text: str);
```

## fun is_accepted

```mach
pub fun is_accepted(s: PhaseStatus) bool;
```

## fun internal_text

```mach
pub fun internal_text(s: PhaseStatus) opt[str];
```

the internal failure's text, absent unless the phase failed internally

## fun status_text

```mach
pub fun status_text(s: PhaseStatus) str;
```

the same as presentation text: a phase that did not fail internally has
nothing to say

## fun phase_failure

```mach
pub fun phase_failure(s: PhaseStatus) Fail;
```

## fun merge

```mach
pub fun merge(into: *PhaseStatus, from: PhaseStatus);
```

## tag Catalog

```mach
pub tag Catalog: u8 {
    malformed:   Member;
    unsupported: Unsupported;
    internal:    Member;
}
```

## fun malformed_member

```mach
pub fun malformed_member(catalog: str, tag: u32) Catalog;
```

a member read from input that the catalog does not declare

## fun malformed_member_in

```mach
pub fun malformed_member_in(catalog: str, tag: u32, where: str) Catalog;
```

the same, named with where the input came from

## fun unsupported_member

```mach
pub fun unsupported_member(catalog: str, member: str, by: str) Catalog;
```

a declared member that `by` (a target, format or phase) does not honor

## fun unknown_member

```mach
pub fun unknown_member(catalog: str, tag: u32) Catalog;
```

a member the compiler produced that its own catalog does not declare

## fun unknown_member_in

```mach
pub fun unknown_member_in(catalog: str, tag: u32, where: str) Catalog;
```

the same, named with the site that produced it

## fun is_internal_member

```mach
pub fun is_internal_member(c: Catalog) bool;
```

## fun catalog_text

```mach
pub fun catalog_text(a: *A.Allocator, c: Catalog) res[str, std_format.FormatError];
```

the message, owned by the caller's allocator (extent str_len + 1, released
with str_free); the only failure a literal format can meet is the allocator's

## fun catalog_interned

```mach
pub fun catalog_interned(itn: *intern.Interner, a: *A.Allocator, c: Catalog) res[str, Fail];
```

the message, owned by the interner so it outlives temporary phase storage,
with `a` as scratch

## fun catalog

```mach
pub fun catalog(a: *A.Allocator, c: Catalog) Fail;
```

the closed-catalog policy as a failure: an input or capability fault is the
user's, a compiler-produced member is internal. the message belongs to the
caller's allocator

## fun unknown_catalog

```mach
pub fun unknown_catalog(a: *A.Allocator, catalog_name: str, tag: u32) Fail;
```

