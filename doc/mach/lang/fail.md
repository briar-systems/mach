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
point at a place, a location of the source map its holder reads, which the
failure is rendered through; an internal one is always `compiler.internal`. the exit
code derives from the case: 1 for user, 2 for internal, 3 where a command
distinguishes the environment. `reported` is declared first so a zero
failure is one that invents no text. an allocation refusal is internal; the
compiler recovers from none of them at the site that met them. a failure
that carries text records whether it owns it: owned text was allocated
through the allocator `dnit` is given and is released there, borrowed text
(a literal, an interned text, storage someone else releases) never is. a
failure that names related places or a fix records whether it owns their
storage in the same way, all of it or none: `places_retain`, `retain`,
`with_related` and `with_fix` own it

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

## fun at

```mach
pub fun at(f: Fail, at: lang_source.Location) Fail;
```

the same failure pointing at `at`, a location of the source map its caller reads. a
failure that already points somewhere keeps its place, the innermost site knowing best;
one with no kind, or a nil `at`, points nowhere. a fix that names no place of its own
rewrites the bytes the failure points at

## fun spanned

```mach
pub fun spanned(f: Fail, s: lang_source.Span) Fail;
```

the same failure pointing at the span `s` of the file that caused it, which `placed`
names, as `at` points it

## fun placed

```mach
pub fun placed(f: Fail, file: lang_source.FileId) Fail;
```

the same failure with every span still waiting for its file, its own, each related one and
each fix edit's, placed in `file`

## fun with_related

```mach
pub fun with_related(a: *A.Allocator, f: Fail, locs: *lang_source.Location, n: usize) Fail;
```

the same failure naming the `n` locations at `locs` as its related places, in order, each
unlabelled; a nil location is skipped. the failure owns its places through `a` as
`places_retain` leaves them. a failure that already names related places keeps them, and one
whose copy is refused is that refusal

## fun with_fix

```mach
pub fun with_fix(a: *A.Allocator, f: Fail, label: str, s: opt[lang_source.Span], replacement: str) Fail;
```

the same failure carrying the rewrite `label` names, which replaces the span `s` of the file
the failure points into, or the bytes it points at when `s` is none, with `replacement`. the
label and the replacement are copied, so the failure owns its places through `a` as
`places_retain` leaves them. a failure with no kind carries none, one that already carries a
fix keeps it, and one whose copy is refused is that refusal

## fun fix_of

```mach
pub fun fix_of(f: Fail) opt[annotation.Fix];
```

the rewrite a failure carries, none when it carries no fix; it shares the failure's places'
lifetime

## fun related_of

```mach
pub fun related_of(f: Fail) Vector[annotation.Related];
```

the related places a failure names, empty for one that names no other; they share the
failure's places' lifetime

## fun borrowing_places

```mach
pub fun borrowing_places(f: Fail) Fail;
```

the same failure borrowing its places, for a view of places a holder owns:
the places it owned before are the caller's to release

## fun place_of

```mach
pub fun place_of(f: Fail) opt[lang_source.Location];
```

where the failure points, when it points at a file of the source map its caller reads

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

a copy of the failure whose places, every related place and its fix, are owned through `a`,
its text left as it is; for a holder that keeps the text apart. released with `places_dnit`

## fun places_dnit

```mach
pub fun places_dnit(a: *A.Allocator, f: Fail);
```

release the places a failure owns through `a`, as `places_retain` made them;
borrowed places are left alone

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

whether two failures hold the same places: the same location and the same related and fix
storage, as a holder that copied them once sees its own copy

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

