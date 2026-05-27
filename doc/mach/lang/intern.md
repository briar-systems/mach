# mach.lang.intern

Identifier interner. Maps text to stable [`StrId`](#strid) handles. The
same byte sequence presented twice resolves to the same
[`StrId`](#strid); the resolved bytes remain valid for the interner's
lifetime.

## Types

### `StrId`

```mach
pub def StrId: u32;
```

Stable handle into an [`Interner`](#interner)'s universe.

### `Interner`

```mach
pub rec Interner {
    alloc:   *Allocator;
    strings: *str;
    len:     usize;
    cap:     usize;
    dedup:   map.Map[str, StrId];
}
```

| Field   | Type                       | Description                                                  |
|---------|----------------------------|--------------------------------------------------------------|
| alloc   | `*Allocator`               | Allocator backing owned bytes, the index, and the dedup map. |
| strings | `*str`                     | Array of owned strings indexed by [`StrId`](#strid).         |
| len     | `usize`                    | Number of strings stored.                                    |
| cap     | `usize`                    | Allocated slots in `strings`.                                |
| dedup   | `map.Map[str, StrId]`      | Content → [`StrId`](#strid) map for O(1) interning.          |

## Constants

```mach
pub val STR_NIL: StrId = 0xFFFFFFFF;
```

Absent-string sentinel.

```mach
val INITIAL_CAP: usize = 32;
```

Starting capacity for [`Interner.strings`](#interner). The array
doubles on overflow.

## Functions

### `init`

```mach
pub fun init(alloc: *Allocator) Interner
```

Constructs an empty interner backed by `alloc`. Infallible.

| Param | Type         | Description                                       |
|-------|--------------|---------------------------------------------------|
| alloc | `*Allocator` | Allocator used for owned strings, index, dedup map.|

Returns the populated [`Interner`](#interner).

### `dnit`

```mach
pub fun dnit(i: *Interner)
```

Frees every owned string, the index array, and the dedup map. `nil` is
a no-op. After `dnit` every [`StrId`](#strid) previously issued is
invalid.

| Param | Type                       | Description                          |
|-------|----------------------------|--------------------------------------|
| i     | [`*Interner`](#interner)   | Interner to tear down. `nil` is a no-op.|

### `intern`

```mach
pub fun intern(i: *Interner, text: str) Result[StrId, str]
```

Returns the [`StrId`](#strid) for `text`. On first occurrence allocates
an owned copy of `text` and a fresh [`StrId`](#strid). On repeat
occurrence returns the cached [`StrId`](#strid) without allocation.
Errors on allocation failure.

| Param | Type                       | Description                          |
|-------|----------------------------|--------------------------------------|
| i     | [`*Interner`](#interner)   | The interner.                        |
| text  | `str`                      | Text to intern.                      |

Returns the [`StrId`](#strid) for `text`, or an allocation error.

### `intern_span`

```mach
pub fun intern_span(i: *Interner, source: str, span: token.Span) Result[StrId, str]
```

Returns the [`StrId`](#strid) for `source[span.offset..span.offset+span.len]`.
Allocates an owned copy only on first occurrence; on repeat occurrence
frees the temporary copy and returns the cached [`StrId`](#strid).
Errors on allocation failure.

| Param  | Type                                  | Description                                |
|--------|---------------------------------------|--------------------------------------------|
| i      | [`*Interner`](#interner)              | The interner.                              |
| source | `str`                                 | Source string the slice is taken from.     |
| span   | [`token.Span`](fe/token.md#span)      | Byte range within `source` to intern.      |

Returns the [`StrId`](#strid) for the substring, or an allocation error.

### `lookup`

```mach
pub fun lookup(i: *Interner, id: StrId) Option[str]
```

Returns the owned text for `id`, or `none` when `id` is out of range.
The returned `str`'s `data` pointer is stable for the interner's
lifetime.

| Param | Type                       | Description                                          |
|-------|----------------------------|------------------------------------------------------|
| i     | [`*Interner`](#interner)   | The interner.                                        |
| id    | [`StrId`](#strid)          | StrId returned by a previous `intern` or `intern_span`.|

Returns `some(text)` when `id` is in range, `none` otherwise.

### `clear`

```mach
pub fun clear(i: *Interner) Result[bool, str]
```

Resets the interner to empty without releasing the backing allocator
or the [`strings`](#interner) slot storage. Frees every owned string
and the dedup map's storage, then re-initialises the dedup map and
sets `len = 0`. Slot capacity stays allocated for reuse.

This is a **full-database-reset** operation: every previously-issued
[`StrId`](#strid) becomes invalid, including ones referenced by live
[`QueryEntry`](query.md#queryentry) results. Callers must invalidate
the entire [`QueryDb`](query.md#querydb) before calling `clear` — using
it between partial recomputes would yank live references out from under
non-invalidated queries. Intended use is workspace-level reset in
long-running hosts (LSP server restart, test runner between independent
suites), not per-recompile cleanup.

| Param | Type                       | Description       |
|-------|----------------------------|-------------------|
| i     | [`*Interner`](#interner)   | Interner to reset.|

Returns `ok(true)` after reset; allocation errors from the new dedup map
propagate as `err`.

## Internal helpers

File-private; listed for reference.

| Function          | Role                                                                          |
|-------------------|-------------------------------------------------------------------------------|
| `store_and_index` | Common path for [`intern`](#intern) and [`intern_span`](#intern_span): appends an owned copy to [`i.strings`](#interner), assigns a fresh [`StrId`](#strid), and registers it in [`i.dedup`](#interner). On allocation failure (strings array or dedup map), frees the candidate before returning so the caller does not double-free. |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`, `std.allocator`,
`std.collections.map`, [`mach.lang.fe.token`](fe/token.md).
