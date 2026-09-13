# mach.lang.diagnostic

## def Severity

```mach
pub def Severity: u8
```

## val SEVERITY_ERROR

```mach
pub val SEVERITY_ERROR:   Severity = 0
```

## val SEVERITY_WARNING

```mach
pub val SEVERITY_WARNING: Severity = 1
```

## val SEVERITY_INFO

```mach
pub val SEVERITY_INFO:    Severity = 2
```

## val SEVERITY_HELP

```mach
pub val SEVERITY_HELP:    Severity = 3
```

## fun severity_valid

```mach
pub fun severity_valid(s: Severity) bool;
```

## def ChildKind

```mach
pub def ChildKind: u8
```

## val CHILD_NOTE

```mach
pub val CHILD_NOTE: ChildKind = 0
```

## val CHILD_HELP

```mach
pub val CHILD_HELP: ChildKind = 1
```

## rec Location

```mach
pub rec Location;
```

## rec Child

```mach
pub rec Child;
```

## rec Related

```mach
pub rec Related;
```

## rec Edit

```mach
pub rec Edit;
```

## rec Fix

```mach
pub rec Fix;
```

## def FixId

```mach
pub def FixId: usize
```

## rec Diagnostic

```mach
pub rec Diagnostic;
```

## rec DiagnosticId

```mach
pub rec DiagnosticId;
```

## rec DiagMark

```mach
pub rec DiagMark;
```

## rec DiagnosticStore

```mach
pub rec DiagnosticStore;
```

## rec DiagnosticBuilder

```mach
pub rec DiagnosticBuilder;
```

## fun store_init

```mach
pub fun store_init(a: *A.Allocator) DiagnosticStore;
```

## fun mark

```mach
pub fun mark(store: *DiagnosticStore) DiagMark;
```

## fun truncate

```mach
pub fun truncate(store: *DiagnosticStore, m: DiagMark) err[fail.Fail];
```

## fun store_dnit

```mach
pub fun store_dnit(store: *DiagnosticStore);
```

## fun len

```mach
pub fun len(store: *DiagnosticStore) usize;
```

## fun get

```mach
pub fun get(store: *DiagnosticStore, index: usize) opt[*Diagnostic];
```

## fun has_errors

```mach
pub fun has_errors(store: *DiagnosticStore) bool;
```

## fun error_count

```mach
pub fun error_count(store: *DiagnosticStore) usize;
```

## fun note_error

```mach
pub fun note_error(store: *DiagnosticStore);
```

## fun builder_init

```mach
pub fun builder_init(a: *A.Allocator, severity: Severity, file_id: source.FileId, span: token.Span, message: str) res[DiagnosticBuilder, fail.Fail];
```

## fun builder_dnit

```mach
pub fun builder_dnit(b: *DiagnosticBuilder);
```

## fun attach_note

```mach
pub fun attach_note(b: *DiagnosticBuilder, text: str) err[fail.Fail];
```

## fun attach_help

```mach
pub fun attach_help(b: *DiagnosticBuilder, text: str) err[fail.Fail];
```

## fun attach_note_committed

```mach
pub fun attach_note_committed(store: *DiagnosticStore, id: DiagnosticId, text: str) err[fail.Fail];
```

## fun attach_help_committed

```mach
pub fun attach_help_committed(store: *DiagnosticStore, id: DiagnosticId, text: str) err[fail.Fail];
```

## fun attach_related

```mach
pub fun attach_related(b: *DiagnosticBuilder, file_id: source.FileId, span: token.Span, label: str) err[fail.Fail];
```

## fun attach_related_committed

```mach
pub fun attach_related_committed(store: *DiagnosticStore, id: DiagnosticId, file_id: source.FileId, span: token.Span, label: str) err[fail.Fail];
```

## fun attach_fix

```mach
pub fun attach_fix(b: *DiagnosticBuilder, label: str, file_id: source.FileId, span: token.Span, replacement: str) res[FixId, fail.Fail];
```

## fun attach_fix_edit

```mach
pub fun attach_fix_edit(b: *DiagnosticBuilder, fix: FixId, file_id: source.FileId, span: token.Span, replacement: str) err[fail.Fail];
```

## fun last_id

```mach
pub fun last_id(store: *DiagnosticStore) opt[DiagnosticId];
```

## fun attach_fix_committed

```mach
pub fun attach_fix_committed(store: *DiagnosticStore, id: DiagnosticId, label: str, file_id: source.FileId, span: token.Span, replacement: str) res[FixId, fail.Fail];
```

## fun attach_fix_edit_committed

```mach
pub fun attach_fix_edit_committed(store: *DiagnosticStore, id: DiagnosticId, fix: FixId, file_id: source.FileId, span: token.Span, replacement: str) err[fail.Fail];
```

## fun remove_fix_committed

```mach
pub fun remove_fix_committed(store: *DiagnosticStore, id: DiagnosticId, fix: FixId) err[fail.Fail];
```

## fun fix_valid

```mach
pub fun fix_valid(fx: *Fix) bool;
```

## fun commit

```mach
pub fun commit(b: *DiagnosticBuilder, store: *DiagnosticStore) res[DiagnosticId, fail.Fail];
```

## fun resolve

```mach
pub fun resolve(store: *DiagnosticStore, id: DiagnosticId) opt[*Diagnostic];
```

absent when the store is nil or the id no longer names a live diagnostic

## val STALE_ID_TEXT

```mach
pub val STALE_ID_TEXT: str = "stale diagnostic id"
```

## fun error

```mach
pub fun error(store: *DiagnosticStore, file_id: source.FileId, span: token.Span, message: str) err[fail.Fail];
```

## fun warning

```mach
pub fun warning(store: *DiagnosticStore, file_id: source.FileId, span: token.Span, message: str) err[fail.Fail];
```

## fun info

```mach
pub fun info(store: *DiagnosticStore, file_id: source.FileId, span: token.Span, message: str) err[fail.Fail];
```

## fun help

```mach
pub fun help(store: *DiagnosticStore, file_id: source.FileId, span: token.Span, message: str) err[fail.Fail];
```

## fun record_error

```mach
pub fun record_error(store: *DiagnosticStore, file_id: source.FileId, span: token.Span, message: str);
```

## fun record_warning

```mach
pub fun record_warning(store: *DiagnosticStore, file_id: source.FileId, span: token.Span, message: str);
```

## fun record_note_committed

```mach
pub fun record_note_committed(store: *DiagnosticStore, id: DiagnosticId, text: str);
```

## fun record_help_committed

```mach
pub fun record_help_committed(store: *DiagnosticStore, id: DiagnosticId, text: str);
```

## fun record_related_committed

```mach
pub fun record_related_committed(store: *DiagnosticStore, id: DiagnosticId, file_id: source.FileId, span: token.Span, label: str);
```

## fun record_fix_committed

```mach
pub fun record_fix_committed(store: *DiagnosticStore, id: DiagnosticId, label: str, file_id: source.FileId, span: token.Span, replacement: str);
```

## fun record_gate_error

```mach
pub fun record_gate_error(store: *DiagnosticStore, itn: *intern.Interner,
file_id: source.FileId, span: token.Span, message: str);
```

## fun record_gate_commit

```mach
pub fun record_gate_commit(store: *DiagnosticStore, itn: *intern.Interner,
b: *DiagnosticBuilder);
```

## fun note_lost

```mach
pub fun note_lost(store: *DiagnosticStore);
```

## fun lost_count

```mach
pub fun lost_count(store: *DiagnosticStore) usize;
```

## val GATE_NEVER_DECIDED_PREFIX

```mach
pub val GATE_NEVER_DECIDED_PREFIX: str =
"this `$if` gate is never decided: `"
```

## val GATE_NEVER_DECIDED_SUFFIX

```mach
pub val GATE_NEVER_DECIDED_SUFFIX: str =
"` is not a compile-time constant anywhere this module can see"
```

## fun gate_error_named

```mach
pub fun gate_error_named(
store: *DiagnosticStore,
itn: *intern.Interner,
file_id: source.FileId,
span: token.Span,
name_id: intern.StrId,
fallback: str) res[bool, fail.Fail];
```

## fun suggestion_build

```mach
pub fun suggestion_build(a: *A.Allocator, name: str) res[str, fail.Fail];
```

## fun fix_label_replace

```mach
pub fun fix_label_replace(a: *A.Allocator, name: str) res[str, fail.Fail];
```

## fun content_equal

```mach
pub fun content_equal(left: *DiagnosticStore, right: *DiagnosticStore) res[bool, fail.Fail];
```

compare observable diagnostic content without allocation or store identity

## fun snapshot_from

```mach
pub fun snapshot_from(src: *DiagnosticStore, from: usize, a: *A.Allocator) res[*DiagnosticStore, fail.Fail];
```

## fun replay_into

```mach
pub fun replay_into(dst: *DiagnosticStore, src: *DiagnosticStore) err[fail.Fail];
```

## rec GateDiagKey

```mach
pub rec GateDiagKey;
```

## fun gate_commit

```mach
pub fun gate_commit(
store: *DiagnosticStore,
itn: *intern.Interner,
b: *DiagnosticBuilder) res[bool, fail.Fail];
```

## fun gate_error

```mach
pub fun gate_error(
store: *DiagnosticStore,
itn: *intern.Interner,
file_id: source.FileId,
span: token.Span,
message: str) res[bool, fail.Fail];
```

