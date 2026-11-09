# mach.lang.fail

## tag Fail

```mach
pub tag Fail: u8 {
    reported;
    message:     Message;
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
compiler recovers from none of them at the site that met them. a failure
that carries text records whether it owns it: owned text was allocated
through the allocator `dnit` is given and is released there, borrowed text
(a literal, an interned text, storage someone else releases) never is. a
failure that points somewhere records whether it owns its places in the same
way, all of them or none: `places_retain`, `retain` and the constructors that
copy a place through an allocator own them, and every constructor that
borrows a place leaves them borrowed

## rec Fix

```mach
pub rec Fix;
```

a machine-applicable rewrite of the file a failure points into: the bytes at
`at`, or at the failure's own place when `at` is the zero place, become
`replacement`. a nil label is no fix. the label and the replacement share the
failure's places' lifetime

label: what the rewrite does, as a fix line names it
at: the bytes it replaces
replacement: the text it writes in their place

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
has no failure to return, when its caller breaks the precondition it states.
the failure borrows `text`; `owning` hands it text it owns

## fun user

```mach
pub fun user(k: diagnostic_kind.Kind, text: str) Fail;
```

the input is wrong or unsupported. a failure names a live row of the
registry: one that names none, or a retired one, is a compiler defect and
becomes an internal failure. the failure borrows `text`

## fun environment

```mach
pub fun environment(k: diagnostic_kind.Kind, text: str) Fail;
```

the machine refused: a file could not be read or written, a tool could not
be spawned. the failure borrows `text`

## fun owning

```mach
pub fun owning(f: Fail) Fail;
```

the same failure owning its text, which was allocated through the allocator
its holder releases it with (`dnit`); a reported failure carries none

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
`located` places them: its own and each related one still without a file.
the failure owns its places through `a` as `located` leaves them. a failure
without a range, or one already resolved, is returned as it is

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
order, each copied through `a`, the failure owning its places as `located`
leaves them; an unspanned place is skipped. a failure that already names
related places keeps them, and one whose copy is refused is that refusal

## fun with_fix

```mach
pub fun with_fix(f: Fail, label: str, s: opt[lang_source.Span], replacement: str) Fail;
```

the same failure carrying the rewrite `label` names, which replaces the span
`s` of the file it points into, or the bytes it points at when `s` is nil, with
`replacement`, both borrowed, so the failure borrows its places: ones it owned
before are the caller's to release. a failure with no kind carries none, and
one that already carries a fix keeps it

## fun fix_of

```mach
pub fun fix_of(f: Fail) opt[Fix];
```

the rewrite a failure carries, none when it carries no fix. a resolved fix
whose own place is the zero place is given the failure's place

## fun related_of

```mach
pub fun related_of(f: Fail) Related;
```

the related places a failure names, none for one that names no other

## fun located

```mach
pub fun located(a: *A.Allocator, f: Fail, at: Place) Fail;
```

the same failure pointing at `at`, its path copied through `a`, so the failure
owns its places through `a`: one that borrowed them owns copies of the rest
too, and one that owned them releases the place `at` replaces. a failure with
no kind points nowhere, and one whose copy is refused is that refusal

## fun borrowing_places

```mach
pub fun borrowing_places(f: Fail) Fail;
```

the same failure borrowing its places, for a view of places a holder owns:
the places it owned before are the caller's to release

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

## fun describe

```mach
pub fun describe(f: Fail) str;
```

## fun with_text

```mach
pub fun with_text(f: Fail, text: str) Fail;
```

the same failure carrying `text` instead, which it borrows: the class, the
kind and the places are kept. text the failure is to own goes through
`owning` as well. the text it carried before is the caller's to release

## fun with_secondary

```mach
pub fun with_secondary(a: *A.Allocator, f: Fail, secondary: Fail) Fail;
```

the same failure also naming `secondary`, a failure met while releasing what
the failing path held (a child cancelled, a file removed, a source closed):
the text gains "; cleanup also failed: " and the secondary's, joined into
`a`, and the class, kind and places stay the failure's. a reported failure
has no text to carry it, so the secondary goes out in its place, the reported
diagnostics already recorded, and a reported secondary already named itself.
a refused join leaves the failure as it was, as `prefixed` does, so the
secondary never takes the failure's place. the joined text is owned through
`a`; a failure returned as it was keeps the ownership it had

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

release what a failure owns through `a`: its text and its places, each when
it owns them

## fun places_retain

```mach
pub fun places_retain(a: *A.Allocator, f: Fail) res[Fail, A.Error];
```

a copy of the failure whose places, its own path and every related place,
and its fix are owned through `a`, its text left as it is; for a holder that
keeps the text apart. released with `places_dnit`

## fun places_dnit

```mach
pub fun places_dnit(a: *A.Allocator, f: Fail);
```

release the places a failure owns through `a`, as `places_retain` made them;
borrowed places are left alone, and a failure carrying no fix owns no fix text

## fun without_places

```mach
pub fun without_places(f: Fail) Fail;
```

the same failure pointing nowhere, naming no related place and carrying no
fix, for a holder whose places were released or never copied

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

## fun prefixed

```mach
pub fun prefixed(a: *A.Allocator, context: str, cause: Fail) Fail;
```

`cause` with `context` in front of its text, joined into `a`, which owns the
new text; the class, the kind and the places are kept. a reported cause has
no text to extend, and a refused join leaves the cause as it was, keeping the
ownership it had, so the context never takes the cause's place

## fun formatted

```mach
pub fun formatted(a: *A.Allocator, fmt: str, va: ...) Fail;
```

an internal failure whose text is `fmt` formatted with `va` into `a`, which
the failure owns; a refusal to format is the failure instead

## fun formatted_user

```mach
pub fun formatted_user(a: *A.Allocator, k: diagnostic_kind.Kind, fmt: str, va: ...) Fail;
```

a user failure under `k` whose text is `fmt` formatted with `va` into `a`,
which the failure owns; a refusal to format is the failure instead

## fun message_of

```mach
pub fun message_of(made: res[str, Fail]) Fail;
```

the internal failure carrying a made text, which it borrows (a made text may
be interned), or the failure that refused to make it

## fun user_of

```mach
pub fun user_of(k: diagnostic_kind.Kind, made: res[str, Fail]) Fail;
```

the user failure under `k` carrying a made text, which it borrows, or the
failure that refused to make it

## fun environment_of

```mach
pub fun environment_of(k: diagnostic_kind.Kind, made: res[str, Fail]) Fail;
```

the environment failure under `k` carrying a made text, which it borrows, or
the failure that refused to make it

## fun refused

```mach
pub fun refused(e: A.Error) Fail;
```

the refusals the compiler recovers from at no site, each an internal failure
carrying its text: the allocator's, a write to a sink of its own, a string
operation, a filesystem step and a format

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

