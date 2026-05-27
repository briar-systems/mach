# mach.lang.diagnostic

Diagnostic sink. A single severity-tagged list of records that every
phase appends to. Render time is decoupled:
[`Diagnostic`](#diagnostic)s carry
`([`FileId`](source.md#fileid), [`Span`](fe/token.md#span))` for source
location and an owned message; rendering to text happens through the
[source registry](source.md).

## Types

### `Severity`

```mach
pub def Severity: u8;
```

Severity level of a [`Diagnostic`](#diagnostic). Ordered most-to-least
urgent.

### `Diagnostic`

```mach
pub rec Diagnostic {
    severity: Severity;
    file_id:  src.FileId;
    span:     token.Span;
    message:  str;
}
```

A single reported diagnostic tied to a span in a source file.

| Field    | Type                                  | Description                                            |
|----------|---------------------------------------|--------------------------------------------------------|
| severity | [`Severity`](#severity)               | One of [`SEVERITY_*`](#constants).                     |
| file_id  | [`src.FileId`](source.md#fileid)      | Identifier of the file the span points into.           |
| span     | [`token.Span`](fe/token.md#span)      | Byte range being reported on.                          |
| message  | `str`                                 | Owned human-readable message.                          |

### `DiagList`

```mach
pub rec DiagList {
    alloc: *Allocator;
    items: *Diagnostic;
    len:   usize;
    cap:   usize;
}
```

Append-only list of diagnostics collected during one or more phases.

| Field | Type                              | Description                                       |
|-------|-----------------------------------|---------------------------------------------------|
| alloc | `*Allocator`                      | Allocator backing the items array and messages.   |
| items | [`*Diagnostic`](#diagnostic)      | Contiguous array of [`Diagnostic`](#diagnostic) records.|
| len   | `usize`                           | Number of diagnostics currently stored.           |
| cap   | `usize`                           | Allocated slots in `items`.                       |

## Constants

```mach
pub val SEVERITY_ERROR:   Severity = 0;
pub val SEVERITY_WARNING: Severity = 1;
pub val SEVERITY_INFO:    Severity = 2;
pub val SEVERITY_HELP:    Severity = 3;
```

[`Severity`](#severity) values, ordered most-to-least urgent.

| Constant            | Value | Meaning                                          |
|---------------------|-------|--------------------------------------------------|
| `SEVERITY_ERROR`    | 0     | A condition that prevents the build from completing.|
| `SEVERITY_WARNING`  | 1     | A non-fatal concern reported to the user.        |
| `SEVERITY_INFO`     | 2     | Neutral information.                             |
| `SEVERITY_HELP`     | 3     | Suggestion or remediation hint.                  |

```mach
val INITIAL_CAP: usize = 16;
```

Starting capacity for [`DiagList.items`](#diaglist). The list doubles
on overflow.

## Functions

### `init`

```mach
pub fun init(alloc: *Allocator) DiagList
```

Constructs an empty diagnostic list backed by `alloc`. Infallible.

| Param | Type         | Description                                  |
|-------|--------------|----------------------------------------------|
| alloc | `*Allocator` | Allocator used for items array and messages. |

Returns the populated [`DiagList`](#diaglist).

### `dnit`

```mach
pub fun dnit(list: *DiagList)
```

Frees every diagnostic's owned message and the items array. `nil` is a
no-op.

| Param | Type                       | Description                            |
|-------|----------------------------|----------------------------------------|
| list  | [`*DiagList`](#diaglist)   | Diag list to tear down. `nil` is a no-op.|

### `error`

```mach
pub fun error(list: *DiagList, file_id: src.FileId, span: token.Span, message: str) Result[bool, str]
```

Appends a [`Diagnostic`](#diagnostic) with `severity = SEVERITY_ERROR`.
`message` is duplicated into `list`'s allocator; the caller retains
ownership of the original.

| Param   | Type                              | Description                                  |
|---------|-----------------------------------|----------------------------------------------|
| list    | [`*DiagList`](#diaglist)          | Diag list to append to.                      |
| file_id | [`src.FileId`](source.md#fileid)  | Identifier of the file the span points into. |
| span    | [`token.Span`](fe/token.md#span)  | Byte range being reported on.                |
| message | `str`                             | Human-readable message; copied into `list`.  |

Returns `ok(true)` on success, or an allocation error.

### `warning`

```mach
pub fun warning(list: *DiagList, file_id: src.FileId, span: token.Span, message: str) Result[bool, str]
```

Appends a [`Diagnostic`](#diagnostic) with `severity = SEVERITY_WARNING`.
Parameters and ownership match [`error`](#error).

### `info`

```mach
pub fun info(list: *DiagList, file_id: src.FileId, span: token.Span, message: str) Result[bool, str]
```

Appends a [`Diagnostic`](#diagnostic) with `severity = SEVERITY_INFO`.
Parameters and ownership match [`error`](#error).

### `help`

```mach
pub fun help(list: *DiagList, file_id: src.FileId, span: token.Span, message: str) Result[bool, str]
```

Appends a [`Diagnostic`](#diagnostic) with `severity = SEVERITY_HELP`.
Parameters and ownership match [`error`](#error).

### `has_errors`

```mach
pub fun has_errors(list: *DiagList) bool
```

Returns `true` when at least one stored [`Diagnostic`](#diagnostic) has
`severity == SEVERITY_ERROR`. Equivalent to
[`has_severity(list, SEVERITY_ERROR)`](#has_severity). The build is
considered failed when this is `true`; warnings and below do not
fail the build by themselves. CLI-side flags such as `-Werror` are the
mechanism for promoting warnings, not changes here.

| Param | Type                       | Description     |
|-------|----------------------------|-----------------|
| list  | [`*DiagList`](#diaglist)   | List to scan.   |

### `has_severity`

```mach
pub fun has_severity(list: *DiagList, threshold: Severity) bool
```

Returns `true` when at least one stored [`Diagnostic`](#diagnostic) has
`severity <= threshold`. Severity values are ordered most-urgent first
(see [Constants](#constants)), so `has_severity(list, SEVERITY_WARNING)`
matches both errors and warnings; `has_severity(list, SEVERITY_HELP)`
matches anything stored.

| Param     | Type                       | Description                                              |
|-----------|----------------------------|----------------------------------------------------------|
| list      | [`*DiagList`](#diaglist)   | List to scan.                                            |
| threshold | [`Severity`](#severity)    | Most-permissive severity that should still count as a hit. |

### `clear`

```mach
pub fun clear(list: *DiagList)
```

Frees every stored diagnostic's owned message and resets `len` to 0
without releasing the items array. Used by long-running hosts (LSP) and
by per-query diagnostic storage when an entry is invalidated.

| Param | Type                       | Description                  |
|-------|----------------------------|------------------------------|
| list  | [`*DiagList`](#diaglist)   | Diag list to empty.          |

## Internal helpers

File-private; listed for reference.

| Function | Role                                                                                                |
|----------|-----------------------------------------------------------------------------------------------------|
| `emit`   | Common path for [`error`](#error) / [`warning`](#warning) / [`info`](#info) / [`help`](#help): duplicates `message` into the list's allocator and appends a fresh [`Diagnostic`](#diagnostic) with the given severity. |

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, `std.allocator`,
[`mach.lang.fe.token`](fe/token.md), [`mach.lang.source`](source.md).
