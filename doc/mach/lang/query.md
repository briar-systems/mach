# mach.lang.query

Demand-driven incremental computation engine. Owns a memoization
database that maps `(QueryKind, key)` pairs to cached values, tracks
the inter-query dependencies that produced each cached value, and
re-runs only the queries whose transitive inputs have changed since
the cached entry was computed. The rest of the compiler — driver,
[`Session`](session.md#session), CLI, tests, eventual LSP — talks to
the compiler only through queries; phases are pure functions of
their explicit inputs to their explicit outputs and never call each
other directly.

Source is `new/lang/query.mach` (currently empty — this spec is the
design intent; implementation lands during the rewrite). The contract
surface here is what every other spec is allowed to depend on; the
storage layout, hashing strategy, and revision representation are
deliberately not pinned in this spec.

## Types

### `QueryKind`

```mach
pub def QueryKind: u16;
```

Discriminator identifying a particular query function. One [`QueryKind`](#querykind)
maps to exactly one pure function `key → value`; the kind value is
also what selects the [`QueryDb`](#querydb)'s per-kind storage shard.

Every kind is either an **input** (its value is set externally and is
the source of truth) or a **derived** query (its value is computed by
a registered function from other queries). Kinds are stable for the
lifetime of a build.

### `Revision`

```mach
pub def Revision: u64;
```

Monotonic counter owned by the [`QueryDb`](#querydb). Bumps on every
[`set_input`](#set_input) call. Cached derived entries record the
revision at which they were computed; a stale entry is one whose
dependencies have a `last_changed` revision newer than the entry's
`computed_at`.

### `QueryDb`

```mach
pub rec QueryDb {
    alloc:    *Allocator;
    revision: Revision;
    storage:  *QueryShard;
    kinds:    u32;
}
```

The full query database. Owns one [`QueryShard`](#queryshard) per
[`QueryKind`](#querykind); the kind is the shard index. Lives on the
[`Session`](session.md#session) so every phase reaches queries through
the session it was handed.

| Field    | Type                                  | Description                                                |
|----------|---------------------------------------|------------------------------------------------------------|
| alloc    | `*Allocator`                          | Backing allocator for shard storage and per-entry buffers. |
| revision | [`Revision`](#revision)               | Current revision — bumped by [`set_input`](#set_input).    |
| storage  | [`*QueryShard`](#queryshard)          | One shard per [`QueryKind`](#querykind); see below.        |
| kinds    | `u32`                                 | Number of registered kinds (length of `storage`).          |

### `ComputeFn`

```mach
pub def ComputeFn: fun(*QueryDb, u64, *Allocator) Result[QueryOutput, str];
```

The signature every derived query's compute function shares — pure
function from `(db, key)` to a [`QueryOutput`](#queryoutput). Uniform
across all derived kinds (the key is always the hashed `u64`, the
output always a byte buffer), so it is a real typed function pointer,
not an erased `*u8`.

### `RevisionFn`

```mach
pub def RevisionFn: fun(u64) Revision;
```

The signature every metadata source's revision-lookup function
shares — maps a key to its current [`Revision`](#revision).

### `QueryShard`

```mach
pub rec QueryShard {
    kind:      QueryKind;
    role:      ShardRole;
    compute:   ComputeFn;
    revisions: RevisionFn;
    entries:   *QueryEntry;
    len:       u32;
    cap:       u32;
}
```

Per-kind storage. Implementation is open (linear scan vs hash table
vs intrusive map); the public contract is that lookups by key are
amortized constant-time and that entries are append-only within a
revision.

| Field     | Type                                       | Description                                                                  |
|-----------|--------------------------------------------|------------------------------------------------------------------------------|
| kind      | [`QueryKind`](#querykind)                  | Self-identifying kind; equals the shard's index in [`QueryDb.storage`](#querydb). |
| role      | [`ShardRole`](#shardrole)                  | Discriminator for input / derived / metadata.                                |
| compute   | [`ComputeFn`](#computefn)                  | Compute function for derived kinds; `nil` for input and metadata.            |
| revisions | [`RevisionFn`](#revisionfn)                | Revision-lookup function for metadata kinds; `nil` for input and derived.    |
| entries   | [`*QueryEntry`](#queryentry)               | Cached entries for this kind, keyed by [`QueryEntry.key`](#queryentry). Empty (and never populated) for metadata kinds. |
| len       | `u32`                                      | Number of entries.                                                           |
| cap       | `u32`                                      | Allocated capacity.                                                          |

### `ShardRole`

```mach
pub def ShardRole: u8;

pub val SHARD_INPUT:    ShardRole = 0;
pub val SHARD_DERIVED:  ShardRole = 1;
pub val SHARD_METADATA: ShardRole = 2;
```

| Constant         | Value | Storage                          | Lookup                                                    |
|------------------|-------|----------------------------------|-----------------------------------------------------------|
| `SHARD_INPUT`    | 0     | Cached values set externally.    | [`get`](#get) returns the cached value.                   |
| `SHARD_DERIVED`  | 1     | Cached values computed on demand.| [`get`](#get) revalidates deps; recomputes if invalid.    |
| `SHARD_METADATA` | 2     | No storage.                      | [`get`](#get) is an error; only used through [`Dep`](#dep). |

### `QueryEntry`

```mach
pub rec QueryEntry {
    key:          u64;
    value:        *u8;
    value_len:    u32;
    computed_at:  Revision;
    last_changed: Revision;
    deps:         *Dep;
    dep_count:    u32;
}
```

One cached `(key, value)` plus the metadata needed to validate it.

| Field        | Type                       | Description                                                                                       |
|--------------|----------------------------|---------------------------------------------------------------------------------------------------|
| key          | `u64`                      | Hashed key. The mapping from a kind's key type to `u64` is the responsibility of the registrar.   |
| value        | `*u8`                      | Opaque buffer holding the value. Layout is the registered query's responsibility.                 |
| value_len    | `u32`                      | Length of `value` in bytes.                                                                       |
| computed_at  | [`Revision`](#revision)    | Revision at which `value` was last recomputed.                                                    |
| last_changed | [`Revision`](#revision)    | Revision at which `value`'s contents last *changed* (not necessarily the same as `computed_at` — recomputes that produce the same bytes do **not** bump this). |
| deps         | [`*Dep`](#dep)             | Queries this entry read while computing.                                                          |
| dep_count    | `u32`                      | Number of entries in `deps`.                                                                      |

### `Dep`

```mach
pub rec Dep {
    kind:         QueryKind;
    key:          u64;
    last_changed: Revision;
}
```

One recorded dependency. The validator compares `last_changed`
against the dep entry's current `last_changed`; equality means the
dep is unchanged and this entry is still valid for that dep.

| Field        | Type                       | Description                                          |
|--------------|----------------------------|------------------------------------------------------|
| kind         | [`QueryKind`](#querykind)  | The dependency's kind.                               |
| key          | `u64`                      | The dependency's hashed key.                         |
| last_changed | [`Revision`](#revision)    | The dep's `last_changed` value at the time *this* entry was computed. |

## Functions

### `init`

```mach
pub fun init(alloc: *Allocator) Result[QueryDb, str]
```

Constructs an empty database. Allocates the shard array lazily as
kinds register; revision starts at 0.

### `dnit`

```mach
pub fun dnit(db: *QueryDb)
```

Releases every shard, every entry, and every entry's `value` /
`deps` buffer. `nil` is a no-op.

### `register_input`

```mach
pub fun register_input(db: *QueryDb, kind: QueryKind) Result[bool, str]
```

Reserves shard `kind` as an input query. Subsequent
[`set_input`](#set_input) calls with this kind are accepted; calls
to [`get`](#get) hit the input's cached value directly without a
compute step.

### `register_derived`

```mach
pub fun register_derived(
    db:      *QueryDb,
    kind:    QueryKind,
    compute: ComputeFn,
) Result[bool, str]
```

Reserves shard `kind` as a derived query and binds its
[`ComputeFn`](#computefn) — see [Derived query contract](#derived-query-contract).

### `register_metadata`

```mach
pub fun register_metadata(
    db:     *QueryDb,
    kind:   QueryKind,
    lookup: RevisionFn,
) Result[bool, str]
```

Reserves shard `kind` as a [metadata source](#canonical-query-catalog).
No compute function is bound; no value is stored. `lookup` is the
[`RevisionFn`](#revisionfn) owned by the external source (e.g.
[`SourceMap`](source.md#sourcemap) for [`Q_FILE_TEXT`](#canonical-query-catalog));
the validator invokes it during dep revalidation. Calling
[`get`](#get) on a metadata kind is an error — consumers reach
metadata revisions through their own [`Dep`](#dep) entries, not by
fetching values.

### `set_input`

```mach
pub fun set_input(
    db:        *QueryDb,
    kind:      QueryKind,
    key:       u64,
    value:     *u8,
    value_len: u32,
) Result[bool, str]
```

Stores a value under an input shard. Bumps
[`QueryDb.revision`](#querydb); if the new bytes differ from the
existing entry's value, the entry's `last_changed` is set to the new
revision (otherwise it stays).

### `invalidate`

```mach
pub fun invalidate(db: *QueryDb, kind: QueryKind, key: u64) Result[bool, str]
```

Drops the cached entry for `(kind, key)` regardless of validity. Used
when a derived query needs to be forced (e.g. a test that wants to
verify the compute path).

### `get`

```mach
pub fun get(db: *QueryDb, kind: QueryKind, key: u64) Result[*QueryEntry, str]
```

The single public lookup. Walks the existing entry (if any), revalidates
its `deps` recursively, and returns a fresh-or-current
[`QueryEntry`](#queryentry). Recomputes are performed by the derived
shard's `compute` function; the function may itself call
[`get`](#get) for further deps, and those `(kind, key, last_changed)`
triples are recorded back into the entry's `deps` list.

Calling [`get`](#get) on a [`SHARD_METADATA`](#shardrole) kind is an
error — metadata kinds have no stored values. Their revisions are
read through the validator when revalidating a dependent's deps.

## Validation

The validity check on a cached entry is:

> For every `d` in `entry.deps`, the current `(d.kind, d.key)` entry's
> `last_changed` must equal `d.last_changed`. If all match, the cached
> `value` is still correct and the recorded `computed_at` is bumped
> to the current revision. Any mismatch triggers a recompute.

Input entries are always "valid" — their `last_changed` is whatever
the most recent [`set_input`](#set_input) set it to. Derived entries
that have never been computed are absent from the shard; [`get`](#get)
runs the compute and inserts them.

[Metadata sources](#canonical-query-catalog) have no entry storage.
When a [`Dep`](#dep) names a metadata kind, the validator calls the
shard's revision-lookup function with the dep's `key`; the returned
revision is compared against `dep.last_changed` the same way as for
regular query deps. Metadata kinds therefore appear in `deps` lists
just like any other kind — the asymmetry is contained inside the
validator.

Recomputation produces a new value buffer. If the new bytes equal the
old, `value`/`last_changed` are unchanged (this is the early-cutoff
that makes `last_changed` distinct from `computed_at`); otherwise
`value` is freed and replaced and `last_changed` is set to the
current revision.

## Derived query contract

A derived compute function has the [`ComputeFn`](#computefn) signature
— `fun(*QueryDb, u64, *Allocator) Result[QueryOutput, str]` — where
[`QueryOutput`](#queryoutput) carries the produced bytes and their
length. The function is a pure function from `(db, key)` to the
output. Its only allowed side effects are:

1. Reading other queries through [`get`](#get) on `db`.
2. Allocating its returned `QueryOutput.bytes` with `alloc`.

It must not stash references to inputs past return, mutate the
database directly, or read external state (filesystem, time, RNG) —
every external input must enter via an input query set by the
driver, CLI, or LSP.

The function is stored directly as a typed
[`ComputeFn`](#computefn) on [`QueryShard.compute`](#queryshard) — no
type erasure, because every derived kind shares this one signature.
The shard's [`role`](#shardrole) is what tells [`get`](#get) whether
to return a cached value, run `compute`, or reject the call.

### `QueryOutput`

```mach
pub rec QueryOutput {
    bytes: *u8;
    len:   u32;
}
```

| Field | Type   | Description                                                |
|-------|--------|------------------------------------------------------------|
| bytes | `*u8`  | Heap-allocated value buffer; ownership transfers into the cache entry. |
| len   | `u32`  | Number of valid bytes.                                     |

## Integration

The query database is the single point of truth for every cached
compiler artifact. Per `new/lang/README.md`, the
[`Session`](session.md#session) owns one [`QueryDb`](#querydb)
instance and is the public surface every consumer (CLI, LSP, tests)
talks to. Phases never call each other directly — they register as
either input or derived queries, and the database does the wiring.

### Canonical query catalog

The kinds below are the load-bearing queries the compiler exposes;
their values are produced by the existing phase modules' result
types ([`ResolveResult`](fe/resolve.md#resolveresult), etc.) and are
serialised into the cache as opaque byte buffers (the registrar is
responsible for the encode / decode pair).

**Inputs** (set by the driver / CLI / LSP via [`set_input`](#set_input)):

| Kind                 | Key                                                    | Value (decoded)                                          |
|----------------------|--------------------------------------------------------|----------------------------------------------------------|
| `Q_PROJECT_ROOT`     | unit (`0`)                                             | [`ProjectConfig`](driver.md#projectconfig) bytes.        |
| `Q_TARGET`           | unit (`0`)                                             | [`target.Target`](target.md#target) bytes.               |

**Metadata sources** (no compute fn, no stored value; the validator
special-cases them to read revisions from an external owner):

| Kind                 | Key                                                    | Revision source                                          |
|----------------------|--------------------------------------------------------|----------------------------------------------------------|
| `Q_FILE_TEXT`        | [`source.FileId`](source.md#fileid)                    | [`SourceFile.revision`](source.md#sourcefile) — read directly from [`SourceMap`](source.md#sourcemap). |

**Derived** (computed on demand):

| Kind                 | Key                                                    | Value (decoded)                                          |
|----------------------|--------------------------------------------------------|----------------------------------------------------------|
| `Q_TOKENIZE`         | [`source.FileId`](source.md#fileid)                    | [`lexer.TokenStream`](fe/lexer.md) bytes.                |
| `Q_PARSE`            | [`source.FileId`](source.md#fileid)                    | [`ast.Ast`](fe/ast.md#ast) bytes.                        |
| `Q_MODULE_ID_FOR_FQN`| [`intern.StrId`](intern.md#strid)                      | [`session.ModuleId`](session.md#moduleid).               |
| `Q_COMPTIME_CTX`     | [`session.ModuleId`](session.md#moduleid)              | [`comptime.ComptimeCtx`](fe/comptime.md#comptimectx) bytes. |
| `Q_RESOLVE`          | [`session.ModuleId`](session.md#moduleid)              | [`resolve.ResolveResult`](fe/resolve.md#resolveresult) bytes. |
| `Q_SEMA`             | [`session.ModuleId`](session.md#moduleid)              | sema output bytes (see [`sema`](fe/sema.md)).            |
| `Q_LOWER`            | [`session.ModuleId`](session.md#moduleid)              | IR module bytes (see [`me.ir`](me/ir.md)).               |
| `Q_CODEGEN`          | [`session.ModuleId`](session.md#moduleid)              | object-module bytes (see [`be.obj`](be/obj.md)).         |
| `Q_LINK`             | unit (`0`)                                             | linked-image bytes (see [`be.linker`](be/linker.md)).    |

The names are spelled `Q_*` to avoid collision with the phase
modules' own type and constant names. Each [`QueryKind`](#querykind)
value is allocated by `session` in a fixed registration order, so
the numeric assignment is stable for a given compiler build.

### Driver as a query consumer

[`driver.build_project`](driver.md#build_project) sets the three
input queries above and then issues a `get` for `Q_LINK` (or for an
intermediate kind, when only a partial build is requested). The
existing DFS load + topological resolve walk becomes the body of
the `Q_RESOLVE` compute path: requesting `Q_RESOLVE(mid)` recursively
requests `Q_RESOLVE` for every dep, and the database memoises each
result. The same applies to subsequent phases.

This contract closes the open audit item: `session.md` and
`driver.md` need their spec text updated to consume the
[`QueryDb`](#querydb) instead of orchestrating the pipeline by
direct calls. The data structures already specced
([`ResolveResult`](fe/resolve.md#resolveresult),
[`comptime.ComptimeCtx`](fe/comptime.md#comptimectx), etc.) survive
unchanged — what changes is *who calls them* and *how memoisation
is threaded*.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, `std.allocator`.
