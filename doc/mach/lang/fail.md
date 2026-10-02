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

the language layer's failure: the phase already recorded it as a diagnostic
on the session and there is nothing further to say; an internal failure
whose text must be preserved; or a failure a pass with no diagnostic store
of its own meets (the linker, an object reader), which is the user's (the
input is wrong or unsupported) or the environment's (a file could not be
read or written) and names the diagnostic kind it is reported as.
`reported` is declared first so a zero outcome is a failure that reports
nothing new, never one that invents text. an allocation refusal is an
internal failure; the compiler recovers from none of them at the site that
met them

## rec Keyed

```mach
pub rec Keyed;
```

kind: the row of the diagnostic kind table the failure is reported as
text: the message

## fun reported

```mach
pub fun reported() Fail;
```

## fun message

```mach
pub fun message(text: str) Fail;
```

an internal failure: a compiler defect, reported as `compiler.internal`

## fun user

```mach
pub fun user(k: dkind.Kind, text: str) Fail;
```

the input is wrong or unsupported; a kind no live row declares makes it
the compiler defect it is

## fun environment

```mach
pub fun environment(k: dkind.Kind, text: str) Fail;
```

the machine refused: a file could not be read or written

## fun kind_of

```mach
pub fun kind_of(f: Fail) dkind.Kind;
```

the kind the failure is reported as, NONE for a reported one, whose
diagnostics carry their own

## fun refused

```mach
pub fun refused(e: std_allocator.Error) Fail;
```

## fun write_refused

```mach
pub fun write_refused(e: io_writer.WriteError) Fail;
```

a refused write met by the compiler (an assembly or diagnostic sink): the
compiler recovers from none of them either, the text names the cause once

## fun str_refused

```mach
pub fun str_refused(e: StrError) Fail;
```

a refused string operation: the allocator's refusal or a slice outside its
string, which is a compiler defect

## fun read_refused

```mach
pub fun read_refused(e: reader.ReadError) Fail;
```

a refused read met by the compiler (an object or archive it was handed)

## fun fs_refused

```mach
pub fun fs_refused(e: std_filesystem.FsError) Fail;
```

a refused filesystem operation met by the compiler: the step that refused
names its own cause

## fun fs_environment

```mach
pub fun fs_environment(k: dkind.Kind, e: std_filesystem.FsError) Fail;
```

a filesystem operation on a file the build was handed (an object, an
archive, a library) that the machine refused: the environment's under `k`,
save an allocation refusal, which stays internal

## fun read_environment

```mach
pub fun read_environment(k: dkind.Kind, e: reader.ReadError) Fail;
```

a read of a file the build was handed that the machine refused, as
`fs_environment`

## fun write_environment

```mach
pub fun write_environment(k: dkind.Kind, e: io_writer.WriteError) Fail;
```

a write of a file the build produces that the machine refused

## fun format_refused

```mach
pub fun format_refused(e: std_format.FormatError) Fail;
```

a refused format: a malformed literal is a compiler defect, the rest is the
sink's or the allocator's refusal

## fun is_reported

```mach
pub fun is_reported(f: Fail) bool;
```

## fun is_message

```mach
pub fun is_message(f: Fail) bool;
```

## val REPORTED_TEXT

```mach
pub val REPORTED_TEXT: str = "failure was reported through diagnostics"
```

the failure as one line of presentation text; a reported failure has no
text of its own and is named as such, never as an empty message

## fun with_text

```mach
pub fun with_text(f: Fail, text: str) Fail;
```

the same failure carrying `text` instead: the class and the kind are kept

## fun describe

```mach
pub fun describe(f: Fail) str;
```

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

## rec Member

```mach
pub rec Member;
```

closed catalogs. a dispatch over a finite catalog that reaches a member no
arm names rejects through one of these, so one message shape names the
catalog and the member and one class says where the fault lies: the member
arrived from input or a cross-module product, the member is valid but the
target or phase declares it cannot honor it, or the compiler produced a
member its own catalog lacks. no site answers an unknown member with a
default.

## rec Unsupported

```mach
pub rec Unsupported;
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
pub fun catalog_text(a: *std_allocator.Allocator, c: Catalog) res[str, std_format.FormatError];
```

the message, owned by the caller's allocator (extent str_len + 1, released
with str_free); the only failure a literal format can meet is the allocator's

## fun catalog_interned

```mach
pub fun catalog_interned(itn: *intern.Interner, a: *std_allocator.Allocator, c: Catalog) res[str, Fail];
```

the message, owned by the interner so it outlives temporary phase storage

## fun catalog_message

```mach
pub fun catalog_message(itn: *intern.Interner, a: *std_allocator.Allocator, c: Catalog) str;
```

the interned message as plain text: an allocation refusal yields its own text,
which is still a failure message and never a valid-looking member

## fun catalog_message_or

```mach
pub fun catalog_message_or(itn: *intern.Interner, a: *std_allocator.Allocator, c: Catalog, generic: str) str;
```

the interned message when the site owns an interner and an allocator, else
`generic`: a static text the caller writes to still name the catalog. a
borrowed view or a test fixture has no owner and still refuses the member

