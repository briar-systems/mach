# mach.lang.fail

## tag Fail

```mach
pub tag Fail: u8 {
    reported;
    message: str;
}
```

the language layer's failure: the phase already recorded it as a diagnostic
on the session and there is nothing further to say, or an internal failure
whose text must be preserved. `reported` is declared first so a zero
outcome is a failure that reports nothing new, never one that invents text.
an allocation refusal is an internal failure; the compiler recovers from
none of them at the site that met them

## fun reported

```mach
pub fun reported() Fail;
```

## fun message

```mach
pub fun message(text: str) Fail;
```

## fun refused

```mach
pub fun refused(e: A.Error) Fail;
```

## fun write_refused

```mach
pub fun write_refused(e: writer.WriteError) Fail;
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
pub fun fs_refused(e: fs.FsError) Fail;
```

a refused filesystem operation met by the compiler: the step that refused
names its own cause

## fun format_refused

```mach
pub fun format_refused(e: format.FormatError) Fail;
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

## fun is_rejected

```mach
pub fun is_rejected(s: PhaseStatus) bool;
```

## fun is_internal

```mach
pub fun is_internal(s: PhaseStatus) bool;
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
pub fun catalog_text(a: *A.Allocator, c: Catalog) res[str, format.FormatError];
```

the message, owned by the caller's allocator (extent str_len + 1, released
with str_free); the only failure a literal format can meet is the allocator's

## fun catalog_interned

```mach
pub fun catalog_interned(itn: *intern.Interner, a: *A.Allocator, c: Catalog) res[str, Fail];
```

the message, owned by the interner so it outlives temporary phase storage

## fun catalog_message

```mach
pub fun catalog_message(itn: *intern.Interner, a: *A.Allocator, c: Catalog) str;
```

the interned message as plain text: an allocation refusal yields its own text,
which is still a failure message and never a valid-looking member

## fun catalog_message_or

```mach
pub fun catalog_message_or(itn: *intern.Interner, a: *A.Allocator, c: Catalog, generic: str) str;
```

the interned message when the site owns an interner and an allocator, else
`generic`: a static text the caller writes to still name the catalog. a
borrowed view or a test fixture has no owner and still refuses the member

## fun catalog_status

```mach
pub fun catalog_status(s: *PhaseStatus, c: Catalog, message: str);
```

the phase status a class lands on: input and capability faults are rejections
the caller reports as a diagnostic, an internal member is an internal failure

## fun io_refused

```mach
pub fun io_refused(e: io_error.Error) Fail;
```

a refused native operation whose text the platform names

## fun parse_refused

```mach
pub fun parse_refused(e: parse.ParseError) Fail;
```

a refused number parse (a target attribute or an inline-asm operand)

## fun encode_refused

```mach
pub fun encode_refused(e: binary.EncodeError) Fail;
```

a refused binary encode (an object writer's encoder)

## fun decode_refused

```mach
pub fun decode_refused(e: binary.DecodeError) Fail;
```

a refused binary decode (an object reader's cursor)

