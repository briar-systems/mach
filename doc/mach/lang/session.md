# mach.lang.session

Top-level service container. Owns the source registry, diagnostic sink,
string interner, and type interner. Every phase takes a
[`*Session`](#session) and routes service interactions through it.

## Types

### `ModuleId`

```mach
pub def ModuleId: u32;
```

Project-wide module handle. Assigned by the driver as modules are
discovered; stable for the lifetime of the build. Distinct from
[`ast.id.ModuleId`](fe/ast/id.md#moduleid), which identifies a `Module`
record within a single [`Ast`](fe/ast.md#ast).

### `Session`

```mach
pub rec Session {
    alloc:    *Allocator;
    sources:  source.SourceMap;
    diags:    diag.DiagList;
    interner: intern.Interner;
    types:    ty.TypeInterner;
}
```

| Field    | Type                                            | Description                                                |
|----------|-------------------------------------------------|------------------------------------------------------------|
| alloc    | `*Allocator`                                    | Backs every owned array on the session and its services.   |
| sources  | [`source.SourceMap`](source.md#sourcemap)       | Source-file registry; [`FileId`](source.md#fileid) handles stable once issued. |
| diags    | [`diag.DiagList`](diagnostic.md#diaglist)       | Append-only diagnostic list.                               |
| interner | [`intern.Interner`](intern.md#interner)         | String interner; [`StrId`](intern.md#strid) handles stable once issued. |
| types    | [`ty.TypeInterner`](type.md#typeinterner)       | Type universe; [`TypeId`](type.md#typeid) handles stable once issued. |

## Constants

```mach
pub val MODULE_NIL: ModuleId = 0xFFFFFFFF;
```

Absent-module sentinel.

## Functions

### `init`

```mach
pub fun init(alloc: *Allocator) Result[Session, str]
```

Constructs a session backed by `alloc`. Sub-services are constructed in
order: `sources`, `diags`, `interner`, `types`. The first three are
infallible. `types` allocates the type universe and pre-populates the
12 primitive [`TypeId`](type.md#typeid)s. On `types` failure, prior
sub-services are torn down in reverse construction order and the error
is returned.

| Param | Type         | Description                                  |
|-------|--------------|----------------------------------------------|
| alloc | `*Allocator` | Allocator used for all session-owned storage.|

Returns the populated [`Session`](#session), or an allocation error from
a fallible sub-service.

### `dnit`

```mach
pub fun dnit(s: *Session)
```

Releases every resource. Sub-services are dnit'd in reverse construction
order (`types`, `interner`, `diags`, `sources`). Passing `nil` is a
no-op.

| Param | Type                       | Description                          |
|-------|----------------------------|--------------------------------------|
| s     | [`*Session`](#session)     | Session to tear down. `nil` is a no-op.|

### `load_source`

```mach
pub fun load_source(s: *Session, path: str, text: str) Result[source.FileId, str]
```

Registers a file with [`sources`](source.md#sourcemap) and returns its
[`FileId`](source.md#fileid). Delegates to [`source.add`](source.md#add).

| Param | Type                       | Description                                    |
|-------|----------------------------|------------------------------------------------|
| s     | [`*Session`](#session)     | Session whose source registry receives the file.|
| path  | `str`                      | File path to associate with this file.         |
| text  | `str`                      | Source text to index and store.                |

Returns the [`FileId`](source.md#fileid) of the newly added file, or an
allocation error.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, `std.allocator`,
[`mach.lang.source`](source.md), [`mach.lang.diagnostic`](diagnostic.md),
[`mach.lang.intern`](intern.md), [`mach.lang.type`](type.md).
