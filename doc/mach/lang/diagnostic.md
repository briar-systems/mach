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
See [`emit`](#emit) for the shared parameter contract.

### `warning`

```mach
pub fun warning(list: *DiagList, file_id: src.FileId, span: token.Span, message: str) Result[bool, str]
```

Appends a [`Diagnostic`](#diagnostic) with `severity = SEVERITY_WARNING`.
See [`emit`](#emit) for the shared parameter contract.

### `info`

```mach
pub fun info(list: *DiagList, file_id: src.FileId, span: token.Span, message: str) Result[bool, str]
```

Appends a [`Diagnostic`](#diagnostic) with `severity = SEVERITY_INFO`.
See [`emit`](#emit) for the shared parameter contract.

### `help`

```mach
pub fun help(list: *DiagList, file_id: src.FileId, span: token.Span, message: str) Result[bool, str]
```

Appends a [`Diagnostic`](#diagnostic) with `severity = SEVERITY_HELP`.
See [`emit`](#emit) for the shared parameter contract.

### `has_errors`

```mach
pub fun has_errors(list: *DiagList) bool
```

Returns `true` when at least one stored [`Diagnostic`](#diagnostic) has
`severity == SEVERITY_ERROR`.

| Param | Type                       | Description     |
|-------|----------------------------|-----------------|
| list  | [`*DiagList`](#diaglist)   | List to scan.   |

### `emit`

```mach
fun emit(list: *DiagList, severity: Severity, file_id: src.FileId, span: token.Span, message: str) Result[bool, str]
```

Internal common implementation used by [`error`](#error),
[`warning`](#warning), [`info`](#info), and [`help`](#help). Duplicates
`message` into the list's allocator and appends a fresh
[`Diagnostic`](#diagnostic). Errors on allocation failure.

| Param    | Type                              | Description                                  |
|----------|-----------------------------------|----------------------------------------------|
| list     | [`*DiagList`](#diaglist)          | Diag list to append to.                      |
| severity | [`Severity`](#severity)           | One of [`SEVERITY_*`](#constants).           |
| file_id  | [`src.FileId`](source.md#fileid)  | Identifier of the file the span points into. |
| span     | [`token.Span`](fe/token.md#span)  | Byte range being reported on.                |
| message  | `str`                             | Human-readable message; copied into `list`.  |

Returns `true` on success, or an allocation error.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, `std.allocator`,
[`mach.lang.fe.token`](fe/token.md), [`mach.lang.source`](source.md).
