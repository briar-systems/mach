# mach.lang.session

Top-level service container and query surface. Owns the source
registry, diagnostic sink, string interner, type interner, and the
[`QueryDb`](query.md#querydb) through which every compiler artifact
is requested. Every phase takes a [`*Session`](#session) — read-only
services (sources / diags / interner / types) are reached as fields;
derived artifacts (parse, resolve, sema, lower, codegen, link) are
reached through the query database. Phases never call each other
directly; their results flow through the query catalog
([query.md → Canonical query catalog](query.md#canonical-query-catalog)).

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
    queries:  query.QueryDb;
}
```

| Field    | Type                                            | Description                                                |
|----------|-------------------------------------------------|------------------------------------------------------------|
| alloc    | `*Allocator`                                    | Backs every owned array on the session and its services.   |
| sources  | [`source.SourceMap`](source.md#sourcemap)       | Source-file registry; [`FileId`](source.md#fileid) handles stable once issued. |
| diags    | [`diag.DiagList`](diagnostic.md#diaglist)       | Append-only diagnostic list.                               |
| interner | [`intern.Interner`](intern.md#interner)         | String interner; [`StrId`](intern.md#strid) handles stable once issued. |
| types    | [`ty.TypeInterner`](type.md#typeinterner)       | Type universe; [`TypeId`](type.md#typeid) handles stable once issued. |
| queries  | [`query.QueryDb`](query.md#querydb)             | Memoization database. Holds every input and every derived compiler artifact. The driver, CLI, LSP, and tests consume the compiler through this surface. |

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

Constructs a session backed by `alloc`. Every sub-service is built
with — and from here on out always sees — `session.alloc`; passing a
different allocator into any sub-service after init is undefined
behaviour. Sub-services are constructed in order: `sources`, `diags`,
`interner`, `types`, `queries`. The first three are infallible. `types` allocates the type universe and
pre-populates the 12 primitive [`TypeId`](type.md#typeid)s. `queries`
allocates the [`QueryDb`](query.md#querydb) and registers every kind
in the [canonical catalog](query.md#canonical-query-catalog) — three
inputs (`Q_PROJECT_ROOT`, `Q_TARGET`, `Q_FILE_TEXT`) plus the derived
kinds (`Q_TOKENIZE` through `Q_LINK`). The compute functions for the
derived kinds are owned by their respective phase modules; the
registration step here just binds the function pointers. On any
sub-service failure, prior sub-services are torn down in reverse
construction order and the error is returned.

| Param | Type         | Description                                  |
|-------|--------------|----------------------------------------------|
| alloc | `*Allocator` | Allocator used for all session-owned storage.|

Returns the populated [`Session`](#session), or an allocation error from
a fallible sub-service.

### `dnit`

```mach
pub fun dnit(s: *Session)
```

Releases every resource. Sub-services are dnit'd in reverse
construction order (`queries`, `types`, `interner`, `diags`,
`sources`). Passing `nil` is a no-op.

| Param | Type                       | Description                          |
|-------|----------------------------|--------------------------------------|
| s     | [`*Session`](#session)     | Session to tear down. `nil` is a no-op.|

### `load_source`

```mach
pub fun load_source(s: *Session, path: str, text: str) Result[source.FileId, str]
```

Registers a file with [`sources`](source.md#sourcemap) and returns its
[`FileId`](source.md#fileid). Delegates to
[`source.add`](source.md#add), which also bumps
[`s.queries.revision`](query.md#querydb) so derived entries depending
on this file invalidate.

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
[`mach.lang.intern`](intern.md), [`mach.lang.type`](type.md),
[`mach.lang.query`](query.md).
