[mach](../README.md) / [base](README.md) / **fail**

# mach.base.fail

the compiler's one failure type, and how a refusal of the machine becomes one

every pass, the linker, the query engine, the session and the build return a `Fail`: reported,
when diagnostics already say it, a compiler defect, the user's input, or the environment's
refusal. a failure may point at a source location and carry related places, notes and a fix,
and it records whether it owns its text and its places. `refusal` is the one way a refused std
operation becomes a failure, naming the operation and the path it acted on.

Source: [`src/base/fail.mach`](../../../src/base/fail.mach)

Uses types from [`mach.base.diagnostic.annotation`](diagnostic/annotation.md), [`mach.base.diagnostic.kind`](diagnostic/kind.md), [`mach.base.source`](source.md).

| Type | | |
| :--- | :--- | :--- |
| [`Fail`](#fail) | `tag` | the compiler's failure, one type for every pass, the linker, the query engine, the session and the build |
| [`Op`](#op) | `rec` | an operation the machine was asked to carry out, as a refusal of it names it: `cannot <verb>[ <role>][ '<path>'][ <link> '<other>'] (<cause>)` |
| [`Cause`](#cause) | `tag` | what refused an operation, as the std operation reported it; `text` is a cause its own module renders (a subprocess, a contained removal) |

| On `Fail` | |
| :--- | :--- |
| [`reported`](#reported) | a failure the diagnostics already say, carrying no text of its own |
| [`message`](#message) | an internal failure: a compiler defect, reported as `compiler.internal` |
| [`user`](#user) | the input is wrong or unsupported |
| [`environment`](#environment) | the machine refused: a file could not be read or written, a tool could not be spawned |
| [`owning`](#owning) | the same failure owning its text, which was allocated through the allocator its holder releases it with (`dnit`); a reported failure carries none |
| [`as_user`](#as_user) | a failure the user caused met by a phase that cannot classify it (target selection, source loading, import libraries): an internal text becomes the user class under `k`, a keyed failure keeps its own class and kind, a reported failure stays reported |
| [`as_internal`](#as_internal) | a failure met on input the compiler produced itself (an object it wrote, a module it emitted): whatever the check says of the input, it is the compiler's defect, an internal failure keeping the text and the place |
| [`defect`](#defect) | the compiler's defect a failure names, borrowing its text and keeping its place, for a holder of the first defect a phase met: an internal failure as it is, a keyed one as the defect its text names, and a reported one as the defect of being handed one, since its diagnostics name no defect of their own |
| [`at`](#at) | the same failure pointing at `at`, a location of the source map its caller reads |
| [`spanned`](#spanned) | the same failure pointing at the span `s` of the file that caused it, which `placed` names, as `at` points it |
| [`placed`](#placed) | the same failure with every span still waiting for its file, its own, each related one and each fix edit's, placed in `file` |
| [`with_related`](#with_related) | the same failure naming the `n` locations at `locs` as its related places, in order, each unlabelled; a nil location is skipped |
| [`with_fix`](#with_fix) | the same failure carrying the rewrite `label` names, which replaces the span `s` of the file the failure points into, or the bytes it points at when `s` is none, with `replacement` |
| [`with_note`](#with_note) | the same failure carrying `text` as a note after any it carries, a line saying more than the failure's text, such as how the input it concerns was reached |
| [`notes_of`](#notes_of) | the notes a failure carries, in order, empty for one that carries none; they share the failure's places' lifetime |
| [`fix_of`](#fix_of) | the rewrite a failure carries, none when it carries no fix; it shares the failure's places' lifetime |
| [`related_of`](#related_of) | the related places a failure names, empty for one that names no other; they share the failure's places' lifetime |
| [`borrowing_places`](#borrowing_places) | the same failure borrowing its places, for a view of places a holder owns: the places it owned before are the caller's to release |
| [`place_of`](#place_of) | where the failure points, when it points at a file of the source map its caller reads |
| [`kind_of`](#kind_of) | the kind the failure is reported as, NONE for a reported one, whose diagnostics carry their own |
| [`is_reported`](#is_reported) | whether the failure was already reported through diagnostics |
| [`is_message`](#is_message) | whether the failure is internal, a compiler defect |
| [`is_environment`](#is_environment) | whether the failure is the environment's refusal |
| [`text`](#text) | the text a failure carries, absent for a reported one |
| [`describe`](#describe) | the failure as one line of presentation text, borrowed from the failure, or REPORTED_TEXT for a reported one |
| [`with_text`](#with_text) | the same failure carrying `text` instead, which it borrows: the class, the kind and the places are kept |
| [`with_secondary`](#with_secondary) | the same failure also naming `secondary`, a failure met while releasing what the failing path held (a child cancelled, a file removed, a source closed): the text gains " |
| [`retain`](#retain) | a copy of the failure that owns its text and its places through `a`, for a failure that outlives the storage its message was made in; released with `dnit` |
| [`dnit`](#dnit) | release what a failure owns through `a`: its text and its places, each when it owns them |
| [`places_retain`](#places_retain) | a copy of the failure whose places, every related place, its notes and its fix, are owned through `a`, its text left as it is; for a holder that keeps the text apart |
| [`places_dnit`](#places_dnit) | release the places a failure owns through `a`, as `places_retain` made them; borrowed places are left alone |
| [`without_places`](#without_places) | the same failure pointing nowhere, naming no related place and carrying no note or fix, for a holder whose places were released or never copied |
| [`same_places`](#same_places) | whether two failures hold the same places: the same location and the same related and fix storage, as a holder that copied them once sees its own copy |
| [`unit`](#unit) | the unit outcome of an operation whose value is not needed |
| [`prefixed`](#prefixed) | `cause` with `context` in front of its text, which follows in parentheses, joined into `a`, which owns the new text; the class, the kind and the places are kept |
| [`formatted`](#formatted) | an internal failure whose text is `fmt` formatted with `va` into `a`, which the failure owns; a refusal to format is the failure instead |
| [`formatted_user`](#formatted_user) | a user failure under `k` whose text is `fmt` formatted with `va` into `a`, which the failure owns; a refusal to format is the failure instead |
| [`message_of`](#message_of) | the internal failure carrying a made text, which it borrows (a made text may be interned), or the failure that refused to make it |
| [`user_of`](#user_of) | the user failure under `k` carrying a made text, which it borrows, or the failure that refused to make it |
| [`environment_of`](#environment_of) | the environment failure under `k` carrying a made text, which it borrows, or the failure that refused to make it |
| [`refused`](#refused) | the allocator's refusal, the environment's, as a failure that carries no text of its own: its message is the allocator's static wording, so it is made, kept, copied and printed with no storage |
| [`str_refused`](#str_refused) | a refused string operation: the allocator's refusal as `refused` gives it, any other internal |
| [`format_refused`](#format_refused) | a refused format: the allocator's refusal as `refused` gives it, any other internal |
| [`source_refused`](#source_refused) | a refusal of the source map: the allocator's refusal as `refused` gives it, any other internal |
| [`toml_refused`](#toml_refused) | a document that does not parse is the user's, pointing at the byte the parser refused; one the allocator refused is the environment's |

| On `Op` | |
| :--- | :--- |
| [`refusal`](#refusal) | the failure of `op`, refused by `cause`, reported under `k` with the text `op` names, allocated in `a`, which the failure borrows |
| [`refusal_text`](#refusal_text) | the text `refusal` names `op` with, allocated in `a`, or the failure that stands in its place, carrying a static text: an allocation refusal behind `cause`, or a refusal to format the text |

| On `Cause` | |
| :--- | :--- |
| [`charged`](#charged) | the failure under `k` carrying `text`, which it borrows, for a refusal of `cause` that a module words itself because its text names more than one operation does (a contained walk names the path, the component and the policy): charged as `refusal` charges it, `given` saying whether the user named the path |
| [`cause_text`](#cause_text) | the text of what refused an operation, for a note or a report line that is not itself a failure; a failure goes through `refusal`, which names the operation |

| Functions | |
| :--- | :--- |
| [`str_text`](#str_text) | the text of a string operation's refusal |
| [`format_text`](#format_text) | the text of a format's refusal |
| [`toml_text`](#toml_text) | the text of a TOML document's refusal |

---

## Fail

```mach
pub tag Fail: u8 {
    reported;
    message:     Message;
    user:        Keyed;
    environment: Keyed;
    refused:     std.allocator.Error;
}
```

the compiler's failure, one type for every pass, the linker, the query
engine, the session and the build. the phase already recorded it as a
diagnostic and there is nothing further to say; an invariant the compiler
owns was broken, whose text must be preserved; the input was wrong or
unsupported; or the machine refused rather than the input (a file that could
not be read or written, a tool that could not be spawned). a user or
environment failure names the diagnostic kind it is reported as, an internal
one is always `compiler.internal`, and any of them may point at a place, a
location of the source map its holder reads, which the failure is rendered
through. the exit
code derives from the case: 1 for user, 2 for internal, 3 where a command
distinguishes the environment. `reported` is declared first so a zero
failure is one that invents no text. an allocation refusal is the
environment's; the compiler recovers from none of them at the site that met
them, and its text is the allocator's own, so it carries none and is made,
kept and copied without an allocation. a failure
that carries text records whether it owns it: owned text was allocated
through the allocator `dnit` is given and is released there, borrowed text
(a literal, an interned text, storage someone else releases) never is. a
failure that names related places, notes or a fix records whether it owns
their storage in the same way, all of it or none: `places_retain`, `retain`,
`with_related`, `with_note` and `with_fix` own it

### reported

```mach
pub fun reported() Fail
```

a failure the diagnostics already say, carrying no text of its own

### message

```mach
pub fun message(text: str) Fail
```

an internal failure: a compiler defect, reported as `compiler.internal`.
this is the one channel for a defect: a function that meets a broken
invariant returns it as a message failure, never as a placeholder, a default
answer or an abort. panic is left to a function documented as total, which
has no failure to return, when its caller breaks the precondition it states.
the failure borrows `text`; `owning` hands it text it owns

### user

```mach
pub fun user(k: mach.base.diagnostic.kind.Kind, text: str) Fail
```

the input is wrong or unsupported. a failure names a live row of the
registry: one that names none, or a retired one, is a compiler defect and
becomes an internal failure. the failure borrows `text`

### environment

```mach
pub fun environment(k: mach.base.diagnostic.kind.Kind, text: str) Fail
```

the machine refused: a file could not be read or written, a tool could not
be spawned. the failure borrows `text`

### owning

```mach
pub fun owning(f: Fail) Fail
```

the same failure owning its text, which was allocated through the allocator
its holder releases it with (`dnit`); a reported failure carries none

### as_user

```mach
pub fun as_user(k: mach.base.diagnostic.kind.Kind, f: Fail) Fail
```

a failure the user caused met by a phase that cannot classify it (target
selection, source loading, import libraries): an internal text becomes the
user class under `k`, a keyed failure keeps its own class and kind, a
reported failure stays reported

### as_internal

```mach
pub fun as_internal(f: Fail) Fail
```

a failure met on input the compiler produced itself (an object it wrote, a
module it emitted): whatever the check says of the input, it is the
compiler's defect, an internal failure keeping the text and the place. a
reported failure stays reported

### defect

```mach
pub fun defect(f: Fail) Fail
```

the compiler's defect a failure names, borrowing its text and keeping its place, for a
holder of the first defect a phase met: an internal failure as it is, a keyed one as the
defect its text names, and a reported one as the defect of being handed one, since its
diagnostics name no defect of their own

### at

```mach
pub fun at(f: Fail, at: mach.base.source.Location) Fail
```

the same failure pointing at `at`, a location of the source map its caller reads. a
failure that already points somewhere keeps its place, the innermost site knowing best;
a reported one, or a nil `at`, points nowhere. a fix that names no place of its own
rewrites the bytes the failure points at

### spanned

```mach
pub fun spanned(f: Fail, s: mach.base.source.Span) Fail
```

the same failure pointing at the span `s` of the file that caused it, which `placed`
names, as `at` points it

### placed

```mach
pub fun placed(f: Fail, file: mach.base.source.FileId) Fail
```

the same failure with every span still waiting for its file, its own, each related one and
each fix edit's, placed in `file`

### with_related

```mach
pub fun with_related(a: *std.allocator.Allocator, f: Fail, locs: *mach.base.source.Location, n: usize) Fail
```

the same failure naming the `n` locations at `locs` as its related places, in order, each
unlabelled; a nil location is skipped. the failure owns its places through `a` as
`places_retain` leaves them. a failure that already names related places keeps them, and one
whose copy is refused is that refusal

### with_fix

```mach
pub fun with_fix(a: *std.allocator.Allocator, f: Fail, label: str, s: opt[mach.base.source.Span], replacement: str) Fail
```

the same failure carrying the rewrite `label` names, which replaces the span `s` of the file
the failure points into, or the bytes it points at when `s` is none, with `replacement`. the
label and the replacement are copied, so the failure owns its places through `a` as
`places_retain` leaves them. a failure with no kind carries none, one that already carries a
fix keeps it, and one whose copy is refused is that refusal

### with_note

```mach
pub fun with_note(a: *std.allocator.Allocator, f: Fail, text: str) Fail
```

the same failure carrying `text` as a note after any it carries, a line saying more than the
failure's text, such as how the input it concerns was reached. the text is copied, so the
failure owns its places through `a` as `places_retain` leaves them. a failure with no kind
carries none, and one whose copy is refused is that refusal

### notes_of

```mach
pub fun notes_of(f: Fail) Vector[mach.base.diagnostic.annotation.Child]
```

the notes a failure carries, in order, empty for one that carries none; they share the
failure's places' lifetime

### fix_of

```mach
pub fun fix_of(f: Fail) opt[mach.base.diagnostic.annotation.Fix]
```

the rewrite a failure carries, none when it carries no fix; it shares the failure's places'
lifetime

### related_of

```mach
pub fun related_of(f: Fail) Vector[mach.base.diagnostic.annotation.Related]
```

the related places a failure names, empty for one that names no other; they share the
failure's places' lifetime

### borrowing_places

```mach
pub fun borrowing_places(f: Fail) Fail
```

the same failure borrowing its places, for a view of places a holder owns:
the places it owned before are the caller's to release

### place_of

```mach
pub fun place_of(f: Fail) opt[mach.base.source.Location]
```

where the failure points, when it points at a file of the source map its caller reads

### kind_of

```mach
pub fun kind_of(f: Fail) mach.base.diagnostic.kind.Kind
```

the kind the failure is reported as, NONE for a reported one, whose
diagnostics carry their own

### is_reported

```mach
pub fun is_reported(f: Fail) bool
```

whether the failure was already reported through diagnostics

### is_message

```mach
pub fun is_message(f: Fail) bool
```

whether the failure is internal, a compiler defect

### is_environment

```mach
pub fun is_environment(f: Fail) bool
```

whether the failure is the environment's refusal

### text

```mach
pub fun text(f: Fail) opt[str]
```

the text a failure carries, absent for a reported one

### describe

```mach
pub fun describe(f: Fail) str
```

the failure as one line of presentation text, borrowed from the failure, or
REPORTED_TEXT for a reported one

### with_text

```mach
pub fun with_text(f: Fail, text: str) Fail
```

the same failure carrying `text` instead, which it borrows: the class, the
kind and the places are kept. text the failure is to own goes through
`owning` as well. the text it carried before is the caller's to release

### with_secondary

```mach
pub fun with_secondary(a: *std.allocator.Allocator, f: Fail, secondary: Fail) Fail
```

the same failure also naming `secondary`, a failure met while releasing what
the failing path held (a child cancelled, a file removed, a source closed):
the text gains ". cleanup also failed (" and the secondary's, joined into
`a`, and the class, kind and places stay the failure's. a reported failure
has no text to carry it, so the secondary goes out in its place, the reported
diagnostics already recorded, and a reported secondary already named itself.
a refused join leaves the failure as it was, as `prefixed` does, so the
secondary never takes the failure's place. the joined text is owned through
`a`; a failure returned as it was keeps the ownership it had

### retain

```mach
pub fun retain(a: *std.allocator.Allocator, f: Fail) res[Fail, std.allocator.Error]
```

a copy of the failure that owns its text and its places through `a`, for a
failure that outlives the storage its message was made in; released with
`dnit`

### dnit

```mach
pub fun dnit(a: *std.allocator.Allocator, f: Fail)
```

release what a failure owns through `a`: its text and its places, each when
it owns them

### places_retain

```mach
pub fun places_retain(a: *std.allocator.Allocator, f: Fail) res[Fail, std.allocator.Error]
```

a copy of the failure whose places, every related place, its notes and its fix, are owned through `a`,
its text left as it is; for a holder that keeps the text apart. released with `places_dnit`

### places_dnit

```mach
pub fun places_dnit(a: *std.allocator.Allocator, f: Fail)
```

release the places a failure owns through `a`, as `places_retain` made them;
borrowed places are left alone

### without_places

```mach
pub fun without_places(f: Fail) Fail
```

the same failure pointing nowhere, naming no related place and carrying no
note or fix, for a holder whose places were released or never copied

### same_places

```mach
pub fun same_places(x: Fail, y: Fail) bool
```

whether two failures hold the same places: the same location and the same related and fix
storage, as a holder that copied them once sees its own copy

### unit

```mach
pub fun unit[V](r: res[V, Fail]) err[Fail]
```

the unit outcome of an operation whose value is not needed

### prefixed

```mach
pub fun prefixed(a: *std.allocator.Allocator, context: str, cause: Fail) Fail
```

`cause` with `context` in front of its text, which follows in parentheses,
joined into `a`, which owns the new text; the class, the kind and the places
are kept. a reported cause has no text to extend, and a refused join leaves
the cause as it was, keeping the ownership it had, so the context never takes
the cause's place

### formatted

```mach
pub fun formatted(a: *std.allocator.Allocator, fmt: str, va: ...) Fail
```

an internal failure whose text is `fmt` formatted with `va` into `a`, which
the failure owns; a refusal to format is the failure instead

### formatted_user

```mach
pub fun formatted_user(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, fmt: str, va: ...) Fail
```

a user failure under `k` whose text is `fmt` formatted with `va` into `a`,
which the failure owns; a refusal to format is the failure instead

### message_of

```mach
pub fun message_of(made: res[str, Fail]) Fail
```

the internal failure carrying a made text, which it borrows (a made text may
be interned), or the failure that refused to make it

### user_of

```mach
pub fun user_of(k: mach.base.diagnostic.kind.Kind, made: res[str, Fail]) Fail
```

the user failure under `k` carrying a made text, which it borrows, or the
failure that refused to make it

### environment_of

```mach
pub fun environment_of(k: mach.base.diagnostic.kind.Kind, made: res[str, Fail]) Fail
```

the environment failure under `k` carrying a made text, which it borrows, or
the failure that refused to make it

### refused

```mach
pub fun refused(e: std.allocator.Error) Fail
```

the allocator's refusal, the environment's, as a failure that carries no text of its own: its
message is the allocator's static wording, so it is made, kept, copied and printed with no storage

| Parameter | |
| :--- | :--- |
| `e` | the allocator's refusal |
| **returns** | the failure |

### str_refused

```mach
pub fun str_refused(e: StrError) Fail
```

a refused string operation: the allocator's refusal as `refused` gives it, any other internal

### format_refused

```mach
pub fun format_refused(e: std.format.FormatError) Fail
```

a refused format: the allocator's refusal as `refused` gives it, any other internal

### source_refused

```mach
pub fun source_refused(e: mach.base.source.Error) Fail
```

a refusal of the source map: the allocator's refusal as `refused` gives it, any other internal

### toml_refused

```mach
pub fun toml_refused(e: toml.error.TomlError) Fail
```

a document that does not parse is the user's, pointing at the byte the parser
refused; one the allocator refused is the environment's

## Op

```mach
pub rec Op {
    verb:  str;
    role:  str;
    path:  str;
    link:  str;
    other: str;
    given: bool;
}
```

an operation the machine was asked to carry out, as a refusal of it names it:
`cannot <verb>[ <role>][ '<path>'][ <link> '<other>'] (<cause>)`. a path the
user never typed (a temporary, a cache entry, a scratch directory) carries the
role it plays, so the failure says what it is for and not only where it is; a
move or a copy names its second path after `link` ("to"); a spawned process
names its command as the path and its working directory as the other, linked
by "in". of two paths the one the user asked about comes first, and a path the
compiler derived (a temporary, a staged copy) is the other; a subject with no path (an environment variable, a stream) is named
by its role alone, and a variable's name is its path

| Field | Type | |
| :--- | :--- | :--- |
| `verb` | `str` | what was being done, without "cannot" |
| `role` | `str` | what the path is for, nil when the path speaks for itself |
| `path` | `str` | the path the operation acted on, nil when the subject has none |
| `link` | `str` | how `other` relates to `path`, nil when there is no second path |
| `other` | `str` | the second path |
| `given` | `bool` | whether the user named `path`, so a path that is not there, is not the kind of entry the operation needs, or is too long, is the user's input and not the machine's refusal |

### refusal

```mach
pub fun refusal(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, op: Op, cause: Cause) Fail
```

the failure of `op`, refused by `cause`, reported under `k` with the text `op`
names, allocated in `a`, which the failure borrows. an allocation refusal is
the environment's `memory.refused` whatever the operation; a path the user
named that is missing or the wrong kind of entry is the user's; every other
refusal is the environment's. this is the one way a refusal of the machine
becomes a failure

### refusal_text

```mach
pub fun refusal_text(a: *std.allocator.Allocator, op: Op, cause: Cause) res[str, Fail]
```

the text `refusal` names `op` with, allocated in `a`, or the failure that
stands in its place, carrying a static text: an allocation refusal behind
`cause`, or a refusal to format the text

## Cause

```mach
pub tag Cause: u8 {
    io:     std.io.error.Error;
    fs:     std.filesystem.FsError;
    native: i64;
    env:    std.process.env.EnvError;
    write:  std.io.writer.WriteError;
    text:   str;
}
```

what refused an operation, as the std operation reported it; `text` is a cause
its own module renders (a subprocess, a contained removal)

### charged

```mach
pub fun charged(k: mach.base.diagnostic.kind.Kind, given: bool, cause: Cause, text: str) Fail
```

the failure under `k` carrying `text`, which it borrows, for a refusal of
`cause` that a module words itself because its text names more than one
operation does (a contained walk names the path, the component and the
policy): charged as `refusal` charges it, `given` saying whether the user
named the path. a failure of a path goes through `refusal` unless its wording
needs this

### cause_text

```mach
pub fun cause_text(c: Cause) str
```

the text of what refused an operation, for a note or a report line that is not
itself a failure; a failure goes through `refusal`, which names the operation

## Functions

### str_text

```mach
pub fun str_text(e: StrError) str
```

the text of a string operation's refusal

### format_text

```mach
pub fun format_text(e: std.format.FormatError) str
```

the text of a format's refusal

### toml_text

```mach
pub fun toml_text(e: toml.error.TomlError) str
```

the text of a TOML document's refusal

