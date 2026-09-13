# mach.lang.query

## def QueryKind

```mach
pub def QueryKind: u16
```

## val Q_PROJECT_ROOT

```mach
pub val Q_PROJECT_ROOT:      QueryKind = 0
```

## val Q_TARGET

```mach
pub val Q_TARGET:            QueryKind = 1
```

## val Q_FILE_TEXT

```mach
pub val Q_FILE_TEXT:         QueryKind = 2
```

## val Q_TOKENIZE

```mach
pub val Q_TOKENIZE:          QueryKind = 3
```

## val Q_PARSE

```mach
pub val Q_PARSE:             QueryKind = 4
```

## val Q_MODULE_ID_FOR_FQN

```mach
pub val Q_MODULE_ID_FOR_FQN: QueryKind = 5
```

## val Q_COMPTIME_CTX

```mach
pub val Q_COMPTIME_CTX:      QueryKind = 6
```

## val Q_RESOLVE

```mach
pub val Q_RESOLVE:           QueryKind = 7
```

## val Q_EXPORTS

```mach
pub val Q_EXPORTS:           QueryKind = 8
```

## val Q_SEMA

```mach
pub val Q_SEMA:              QueryKind = 9
```

## val Q_LOWER

```mach
pub val Q_LOWER:             QueryKind = 10
```

## val Q_CODEGEN

```mach
pub val Q_CODEGEN:           QueryKind = 11
```

## val Q_LINK

```mach
pub val Q_LINK:              QueryKind = 12
```

## val Q_TYPED_EXPORTS

```mach
pub val Q_TYPED_EXPORTS:     QueryKind = 13
```

## val Q_MODULE_NUMBER

```mach
pub val Q_MODULE_NUMBER:     QueryKind = 14
```

## val Q_LINK_CONFIG

```mach
pub val Q_LINK_CONFIG:       QueryKind = 15
```

## val Q_LOWERED_SURFACE

```mach
pub val Q_LOWERED_SURFACE:   QueryKind = 17
```

## val Q_EMBED_FILE

```mach
pub val Q_EMBED_FILE:        QueryKind = 18
```

## val Q_CODEGEN_FLAGS

```mach
pub val Q_CODEGEN_FLAGS:     QueryKind = 19
```

## val Q_GATE_TERMINAL

```mach
pub val Q_GATE_TERMINAL:     QueryKind = 20
```

## val Q_INLINE_BODIES

```mach
pub val Q_INLINE_BODIES:     QueryKind = 21
```

## val Q_CELL_SNAPSHOT

```mach
pub val Q_CELL_SNAPSHOT:     QueryKind = 22
```

## def Revision

```mach
pub def Revision: u64
```

## def ShardRole

```mach
pub def ShardRole: u8
```

## val SHARD_INPUT

```mach
pub val SHARD_INPUT:    ShardRole = 0
```

## val SHARD_DERIVED

```mach
pub val SHARD_DERIVED:  ShardRole = 1
```

## val SHARD_METADATA

```mach
pub val SHARD_METADATA: ShardRole = 2
```

## def FinalizeFn

```mach
pub def FinalizeFn: fun(*u8, u32, *A.Allocator)
```

## rec MetadataRevision

```mach
pub rec MetadataRevision;
```

## def RevisionFn

```mach
pub def RevisionFn: fun(ptr, u64) res[MetadataRevision, fail.Fail]
```

## rec RevisionProvider

```mach
pub rec RevisionProvider;
```

## rec QueryKey

```mach
pub rec QueryKey;
```

## rec Operation

```mach
pub rec Operation;
```

a handle on one query operation: the span between begin and end in which products are
computed and validated. it is stale once end ran and collect or discard released the
presentation; every entry point that takes one refuses a stale or foreign handle

owner: the QueryDb that issued it
sequence: the operation number, unique within the db

## rec PreparedQuery

```mach
pub rec PreparedQuery;
```

## rec Presentation

```mach
pub rec Presentation;
```

what collect hands the caller after an operation: the diagnostics of every product
reachable from the roots, copied into a store the caller's allocator owns, and the
operation's failure if it had one. released with presentation_dnit, exactly once

alloc: owns the presentation and its store
diags: the collected diagnostics, in dependency order, each product once
failure: the operation's failure; absent when every product computed
message: the failure's text when one was recorded, owned by the db's allocator

## rec QueryOutput

```mach
pub rec QueryOutput;
```

## rec Dep

```mach
pub rec Dep;
```

## rec QueryView

```mach
pub rec QueryView;
```

a borrowed view of a published product's bytes. valid only inside the operation that
produced it and until the next set_input, invalidate, retirement or teardown of the db;
the caller never frees it

value: the bytes the product published, owned by the db or the product's finalizer
value_len: their length
last_changed: the revision the bytes last changed at

## rec QueryEntryDomain

```mach
pub rec QueryEntryDomain;
```

## rec QueryShard

```mach
pub rec QueryShard;
```

## rec QueryDb

```mach
pub rec QueryDb;
```

## rec QueryCapabilities

```mach
pub rec QueryCapabilities[T];
```

## rec QueryRuntime

```mach
pub rec QueryRuntime[T];
```

## fun init

```mach
pub fun init(a: *A.Allocator) res[QueryDb, fail.Fail];
```

a database over a caller-owned allocator; register every kind before the first begin
and tear down with dnit, which runs every owned product's finalizer

a: owns the db, its shards and every derived product's bytes

## fun dnit

```mach
pub fun dnit(db: *QueryDb);
```

tear the database down: every owned product's finalizer runs on its bytes, every shard
and the operation state are released. nil is a no-op

## fun register_input

```mach
pub fun register_input(db: *QueryDb, kind: QueryKind) err[fail.Fail];
```

## fun register_derived

```mach
pub fun register_derived(db: *QueryDb, kind: QueryKind) err[fail.Fail];
```

## fun register_derived_owned

```mach
pub fun register_derived_owned(db: *QueryDb, kind: QueryKind, finalize: FinalizeFn) err[fail.Fail];
```

## fun register_derived_equatable

```mach
pub fun register_derived_equatable(db: *QueryDb, kind: QueryKind, finalize: FinalizeFn,
equal: fun(*u8, u32, *u8, u32) bool) err[fail.Fail];
```

## fun register_metadata

```mach
pub fun register_metadata(db: *QueryDb, kind: QueryKind, provider: RevisionProvider) err[fail.Fail];
```

## fun runtime

```mach
pub fun runtime[T](db: *QueryDb, compute: fun(*T, QueryKind, u64, *A.Allocator, *diagnostic.DiagnosticStore) res[QueryOutput, fail.Fail]) res[QueryRuntime[T], fail.Fail];
```

## fun begin

```mach
pub fun begin(db: *QueryDb) res[Operation, fail.Fail];
```

open an operation. one is open at a time, and the previous one's presentation must
have been collected or discarded first; the validated set and metadata samples of the
last operation are cleared

db: the database
ret: the handle every get, prepare and end of this operation takes

## fun end

```mach
pub fun end(db: *QueryDb, operation: Operation) err[fail.Fail];
```

close an operation: no computation may be on the stack and no prepared product may be
unfinished. the operation's presentation becomes pending and must be collected or
discarded before the next begin. err carries the failure the operation recorded, or a
metadata provider that changed under it; the presentation is pending either way

db: the database
operation: the handle begin returned

## fun discard

```mach
pub fun discard(db: *QueryDb, operation: Operation) err[fail.Fail];
```

release a pending presentation without collecting it: the attempts, the failure and
its message are dropped and the db accepts the next begin

db: the database
operation: the ended operation's handle

## fun take_failure_message

```mach
pub fun take_failure_message(db: *QueryDb, operation: Operation) res[str, fail.Fail];
```

take ownership of the ended operation's failure text before discarding it; the string
is allocated by the db's allocator and the caller frees it. nil when none was recorded

db: the database
operation: the ended operation's handle

## fun collect

```mach
pub fun collect(db: *QueryDb, operation: Operation, roots: *QueryKey, count: usize,
a: *A.Allocator) res[*Presentation, fail.Fail];
```

collect the ended operation's presentation: the diagnostics of every product reachable
from the roots through recorded dependencies, each once, plus the operation's failure
and message, then discard the pending state. on success the operation is over; on a
failure nothing was released and the presentation is still pending

db: the database
operation: the ended operation's handle
roots: the products to walk from; nil only with count 0
count: how many roots
a: owns the returned presentation
ret: the presentation, released with presentation_dnit, or the copy failure

## fun presentation_dnit

```mach
pub fun presentation_dnit(p: *Presentation);
```

release a presentation, its store and its failure text; nil is a no-op

## fun is_registered

```mach
pub fun is_registered(db: *QueryDb, kind: QueryKind) bool;
```

## fun set_input

```mach
pub fun set_input(db: *QueryDb, kind: QueryKind, key: u64, value: *u8, value_len: u32) err[fail.Fail];
```

## fun invalidate

```mach
pub fun invalidate(db: *QueryDb, kind: QueryKind, key: u64) err[fail.Fail];
```

## rec PreparedRetirement

```mach
pub rec PreparedRetirement;
```

## fun discard_retirement

```mach
pub fun discard_retirement(prepared: *PreparedRetirement);
```

## fun prepare_retirement

```mach
pub fun prepare_retirement(db: *QueryDb, roots: *QueryKey, count: usize) res[PreparedRetirement, fail.Fail];
```

## fun validate_retirement

```mach
pub fun validate_retirement(prepared: *PreparedRetirement) err[fail.Fail];
```

## fun commit_retirement

```mach
pub fun commit_retirement(prepared: *PreparedRetirement) err[fail.Fail];
```

## fun get

```mach
pub fun get[T](rt: *QueryRuntime[T], ctx: *T, kind: QueryKind, key: u64) res[QueryView, fail.Fail];
```

successful end permits serial reads until the next begin, input mutation, or retirement
the current value of a product, computing or revalidating it inside the open operation
and recording it as a dependency of the product being computed, if any

rt: the runtime whose compute callback builds derived products
ctx: the callback's context
kind: the registered kind
key: the product's key within the kind
ret: a borrowed view (QueryView's lifetime), or the product's failure

## fun reusable

```mach
pub fun reusable[T](rt: *QueryRuntime[T], ctx: *T, kind: QueryKind, key: u64) res[bool, fail.Fail];
```

validation may refresh dependencies but never computes the requested product

## fun prepare

```mach
pub fun prepare[T](rt: *QueryRuntime[T], ctx: *T, kind: QueryKind, key: u64,
compute: fun(*T, QueryKind, u64, *A.Allocator, *diagnostic.DiagnosticStore) res[QueryOutput, fail.Fail]) res[PreparedQuery, fail.Fail];
```

## fun prepared_view

```mach
pub fun prepared_view(db: *QueryDb, prepared: PreparedQuery) res[QueryView, fail.Fail];
```

## fun cancel_prepared

```mach
pub fun cancel_prepared(db: *QueryDb, prepared: PreparedQuery) err[fail.Fail];
```

## fun complete_prepared

```mach
pub fun complete_prepared(db: *QueryDb, prepared: PreparedQuery, result: err[fail.Fail]) err[fail.Fail];
```

## fun revision

```mach
pub fun revision[T](rt: *QueryRuntime[T], ctx: *T, kind: QueryKind, key: u64) res[Revision, fail.Fail];
```

## fun depend

```mach
pub fun depend[T](rt: *QueryRuntime[T], ctx: *T, kind: QueryKind, key: u64) err[fail.Fail];
```

## fun observe

```mach
pub fun observe(db: *QueryDb, kind: QueryKind, key: u64) res[MetadataRevision, fail.Fail];
```

## fun cached_dependency_keys

```mach
pub fun cached_dependency_keys(db: *QueryDb, kind: QueryKind, key: u64,
a: *A.Allocator) res[Vector[QueryKey], fail.Fail];
```

observation copies recorded keys without validating or exposing a cached product

## fun peek_revision

```mach
pub fun peek_revision(db: *QueryDb, kind: QueryKind, key: u64) Revision;
```

## fun shard_len

```mach
pub fun shard_len(db: *QueryDb, kind: QueryKind) u32;
```

