[mach](../README.md) / [session](README.md) / **query**

# mach.session.query

the query database: every product is of one kind, a `Kind` row its owner
declares beside the computation. a row says how its bytes are owned (an input
the host sets, derived bytes the database owns, or an owned handle released by
its finalizer), what its keys name and how a derived product is computed; the
database takes a row on its first use and knows no kind of its own. a product
records its dependencies by reading them inside its compute (`get`, `depend`),
and owns the diagnostic store its compute wrote into. validation is transitive
and depth first; equal bytes on recomputation keep the previous revision and
stop propagation; a failed dependency fails the product and the previous value
is never served. all reads happen inside one operation

Source: [`src/session/query.mach`](../../../src/session/query.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`KeyOwner`](#keyowner) | `def` | what a kind's keys name, so a session can release every product a module or file left behind |
| [`Revision`](#revision) | `def` |  |
| [`Role`](#role) | `def` | how a kind's products come to be: set by the host or computed reached through Kind.role |
| [`FinalizeFn`](#finalizefn) | `def` | releases what an owned product's bytes hold reached through Kind.finalize |
| [`EqualFn`](#equalfn) | `def` | whether two owned products are the same reached through Kind.equal |
| [`ComputeFn`](#computefn) | `def` | a derived product's computation: the context the operation's runtime carries, the row it computes, the key, the allocator its bytes come from and the store it reports into reached through Kind.compute |
| [`Kind`](#kind) | `rec` | one kind of product, declared once beside the computation that owns it |
| [`QueryKey`](#querykey) | `rec` | a product's address: the row of its kind and its key |
| [`Operation`](#operation) | `rec` | a handle on one query operation: the span between begin and end in which products are computed and validated |
| [`PreparedQuery`](#preparedquery) | `rec` | a handle on a product prepare computed, which complete_prepared publishes or cancel_prepared drops |
| [`Presentation`](#presentation) | `rec` | what collect hands the caller after an operation: the diagnostics of every product reachable from the roots, copied into a store the caller's allocator owns, and the operation's failure if it had one |
| [`QueryOutput`](#queryoutput) | `rec` | the bytes a derived product's computation publishes, allocated from the allocator it was handed |
| [`QueryView`](#queryview) | `rec` | a borrowed view of a published product's bytes |
| [`QueryDb`](#querydb) | `rec` | the query database: one shard per kind, the open operation's state and the failures it holds |
| [`QueryRuntime`](#queryruntime) | `rec` | the database as an operation's computations see it: each product is computed with the runtime's context, a `T` the computations of every kind read through |
| [`PreparedRetirement`](#preparedretirement) | `rec` | the products a retirement removes, valid while the database stays as it was prepared against |
| [`ProductFilter`](#productfilter) | `def` | whether a product walk enters a product, visiting it and walking what it was computed from reached through walk_products |
| [`ProductVisit`](#productvisit) | `def` | what a product walk is shown of each product: its address and the store it reported into, nil when it reported nothing reached through walk_products |
| [`Read`](#read) | `rec` | a computation that reads off the stack |
| [`ReadLog`](#readlog) | `rec` | the reads of one computation in the order it made them, those before `done` replayed |
| [`ReadFn`](#readfn) | `def` | how a computation reads a product: through `get` on the owner, or through views it logs reached through Reads.read |
| [`Reads`](#reads) | `rec` | how one computation reads products |
| [`Views`](#views) | `rec` | products a reading may read, each validated in this operation, ready and not given back |

| On `KeyOwner` | |
| :--- | :--- |
| [`KEY_UNOWNED`](#keyowner), [`KEY_MODULE`](#keyowner), [`KEY_FILE`](#keyowner) | |

| On `Role` | |
| :--- | :--- |
| [`INPUT`](#role), [`DERIVED`](#role) | |

| On `Presentation` | |
| :--- | :--- |
| [`presentation_dnit`](#presentation_dnit) | release a presentation, its store and its failure text; nil is a no-op |

| On `QueryDb` | |
| :--- | :--- |
| [`init`](#init) | a database over a caller-owned allocator, holding no kind until a row is first used; torn down with dnit, which runs every owned product's finalizer |
| [`failure_dnit`](#failure_dnit) | release the text and the places of the last failure the database holds |
| [`adopt_failure`](#adopt_failure) | hold the failure an operation handed back, its text and places owned through the query database, as the held failure, replacing what the slot held; the database owns it from here |
| [`hold_failure`](#hold_failure) | hold a failure where the database presents it: its text and the places it points at are copied into the database's slot, replacing what the slot held, and the failure returned reads the held copies |
| [`origin_of`](#origin_of) | the origin of the product that raised `f`: the operation's, the held failure's or the last presentation's when `f` reads that failure's text, ORIGIN_NONE for any other failure and for one raised outside every computation |
| [`presented_dnit`](#presented_dnit) | release the last operation's presentation and the failure held beside it |
| [`dnit`](#dnit) | tear the database down: every owned product's finalizer runs on its bytes, every shard and the operation state are released |
| [`runtime`](#runtime) | the runtime an operation's computations read the database through |
| [`begin`](#begin) | open an operation |
| [`end`](#end) | close an operation: no computation may be on the stack and no prepared product may be unfinished |
| [`discard`](#discard) | release a pending presentation without collecting it: the attempts, the failure and its message are dropped and the db accepts the next begin |
| [`take_failure`](#take_failure) | take ownership of the ended operation's failure before discarding it: its text and places are owned through the db's allocator and the caller frees them, and what borrowed them stays valid |
| [`collect`](#collect) | collect the ended operation's presentation: the diagnostics of every product reachable from the roots through recorded dependencies, each once, plus the operation's failure, then discard the pending state |
| [`set_input`](#set_input) | set an input product's bytes, copied into the database |
| [`prepare_retirement`](#prepare_retirement) | a retirement rooted at the given products and every product that depends on them |
| [`prepare_owned_retirement`](#prepare_owned_retirement) | a retirement rooted at every product whose key a released module or file owns, with its dependents |
| [`prepared_held`](#prepared_held) | whether the prepared query's compute failed, its failure held for complete_prepared to record |
| [`preparing`](#preparing) | whether the computation on the stack is the prepare of the product `kind` keys under `key`, which a compute reads to tell a prepare that finishes it outside the computation from a read that does not |
| [`prepared_view`](#prepared_view) | a view of a prepared product's bytes, before they are published |
| [`prepared_diags`](#prepared_diags) | the store a prepared query's compute reports into: work finished outside the compute (a worker thread's optimize) appends there, so a refusal it records reaches the presentation like one the compute made itself |
| [`cancel_prepared`](#cancel_prepared) | drop a prepared product unpublished, keeping its dependencies and diagnostics as the attempt's evidence |
| [`complete_prepared`](#complete_prepared) | publish a prepared product, or record its failure: the one its compute raised, held until now, else `result`, the failure of the work finished outside the computation |
| [`walk_products`](#walk_products) | visit a product and every product it was computed from that the filter enters, each once and its dependencies first, as presentation walks them |
| [`peek_revision`](#peek_revision) | the revision a product last changed at, without validating it; 0 when it has none |
| [`published`](#published) | a product as its last successful computation published it, read between operations by the host that ran them |
| [`value_dnit`](#value_dnit) | give back the value of a derived product no reader of the build reads again: its finalizer runs and its bytes go, while the entry stays current, so a product computed from it still validates against its revision |
| [`views_add`](#views_add) | every product of `kind` validated in this operation added to `v` |
| [`views_add_deps`](#views_add_deps) | every product of `kind` the product `of` keyed `key` read, validated in this operation, added to `v` |
| [`replay`](#replay) | the reads `log` holds past `done` replayed onto the computing frame, as `get` records a read of a validated product: the edge, then its revisions |

| On `QueryRuntime` | |
| :--- | :--- |
| [`get`](#get) | the current value of a product, computing or revalidating it inside the open operation and recording it as a dependency of the product being computed, if any |
| [`reusable`](#reusable) | validation may refresh dependencies but never computes the requested product |
| [`prepare`](#prepare) | compute a derived product the caller finishes outside the computation (a worker thread), published by complete_prepared or dropped by cancel_prepared before the operation ends |
| [`revision`](#revision-1) | the revision a product last changed at, read as `get` reads it |
| [`depend`](#depend) | record a dependency on a product without reading its bytes, validating or computing it as `get` does |

| On `PreparedRetirement` | |
| :--- | :--- |
| [`discard_retirement`](#discard_retirement) | release a retirement that was prepared and not committed; one already committed or discarded owns nothing |
| [`validate_retirement`](#validate_retirement) | whether a prepared retirement still matches its database |
| [`commit_retirement`](#commit_retirement) | remove every selected product, its finalizer run on its bytes, and release the retirement a retirement that removes anything takes a new revision |

| On `ReadLog` | |
| :--- | :--- |
| [`log_init`](#log_init) | an empty read log |
| [`log_dnit`](#log_dnit) | release a read log |

| On `Views` | |
| :--- | :--- |
| [`views_init`](#views_init) | an empty view set |
| [`views_dnit`](#views_dnit) | release a view set, leaving its base alone |
| [`view_read`](#view_read) | the product `kind` keys under `key` read through `v` and logged, or none when `v` does not hold it |

---

## KeyOwner

```mach
pub def KeyOwner: u8
```

what a kind's keys name, so a session can release every product a module or file left behind.
a kind keyed by a module carries its stable module id in the key's low 32 bits, whatever the
higher bits add (a test build), and a kind keyed by a file is keyed by its file id alone. a row left at KEY_UNDECLARED is refused on its first use, so a kind cannot leak by omission

| Name | Value | |
| :--- | :--- | :--- |
| `KEY_UNOWNED` | `1` |  |
| `KEY_MODULE` | `2` |  |
| `KEY_FILE` | `3` |  |

## Revision

```mach
pub def Revision: u64
```

## Role

```mach
pub def Role: u8
```

how a kind's products come to be: set by the host or computed
reached through Kind.role

| Name | Value | |
| :--- | :--- | :--- |
| `INPUT` | `1` |  |
| `DERIVED` | `2` |  |

## FinalizeFn

```mach
pub def FinalizeFn: fun(*u8, u32, *std.allocator.Allocator)
```

releases what an owned product's bytes hold
reached through Kind.finalize

## EqualFn

```mach
pub def EqualFn: fun(*u8, u32, *u8, u32) bool
```

whether two owned products are the same
reached through Kind.equal

## ComputeFn

```mach
pub def ComputeFn: fun(ptr, *Kind, u64, *std.allocator.Allocator, *mach.base.diagnostic.DiagnosticStore) res[QueryOutput, mach.base.fail.Fail]
```

a derived product's computation: the context the operation's runtime carries, the row it
computes, the key, the allocator its bytes come from and the store it reports into
reached through Kind.compute

## Kind

```mach
pub rec Kind {
    name:     str;
    role:     Role;
    owner:    KeyOwner;
    origin:   mach.base.diagnostic.Origin;
    compute:  ComputeFn;
    finalize: FinalizeFn;
    equal:    EqualFn;
    replay:   bool;
}
```

one kind of product, declared once beside the computation that owns it. the database takes
the row on its first use; a row's address is its identity, so two rows never share a shard

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `str` | how failures and cycles name the kind |
| `role` | `Role` | INPUT or DERIVED |
| `owner` | `KeyOwner` | what the kind's keys name |
| `origin` | `mach.base.diagnostic.Origin` | the phase a derived product's diagnostics belong to |
| `compute` | `ComputeFn` | a derived product's computation; an input has none |
| `finalize` | `FinalizeFn` | releases what an owned derived product's bytes hold, called only on bytes its own computation published; nil when the bytes own nothing |
| `equal` | `EqualFn` | whether two owned products are the same, so an equal recomputation keeps the revision; requires finalize |
| `replay` | `bool` | the product lowers or generates a module's object, so a restored object replays its warnings even when the module's front end ran again and reported its own |

## QueryKey

```mach
pub rec QueryKey {
    kind: *Kind;
    key:  u64;
}
```

a product's address: the row of its kind and its key

## Operation

```mach
pub rec Operation {
    owner:    ptr;
    sequence: u64;
}
```

a handle on one query operation: the span between begin and end in which products are
computed and validated. it is stale once end ran and collect or discard released the
presentation; every entry point that takes one refuses a stale or foreign handle

| Field | Type | |
| :--- | :--- | :--- |
| `owner` | `ptr` | the QueryDb that issued it |
| `sequence` | `u64` | the operation number, unique within the db |

## PreparedQuery

```mach
pub rec PreparedQuery {
    operation: Operation;
    key:       Slot;
}
```

a handle on a product prepare computed, which complete_prepared publishes or cancel_prepared drops

| Field | Type | |
| :--- | :--- | :--- |
| `operation` | `Operation` | the operation it was prepared in |
| `key` | `Slot` | the product's address in the database |

## Presentation

```mach
pub rec Presentation {
    alloc:         *std.allocator.Allocator;
    diags:         mach.base.diagnostic.DiagnosticStore;
    failure:       opt[mach.base.fail.Fail];
    failure_alloc: *std.allocator.Allocator;
    origin:        mach.base.diagnostic.Origin;
}
```

what collect hands the caller after an operation: the diagnostics of every product
reachable from the roots, copied into a store the caller's allocator owns, and the
operation's failure if it had one. released with presentation_dnit, exactly once

| Field | Type | |
| :--- | :--- | :--- |
| `alloc` | `*std.allocator.Allocator` | owns the presentation and its store |
| `diags` | `mach.base.diagnostic.DiagnosticStore` | the collected diagnostics, in dependency order, each product once |
| `failure` | `opt[mach.base.fail.Fail]` | the operation's failure, owning its text and places through `failure_alloc`; absent when every product computed |
| `origin` | `mach.base.diagnostic.Origin` | the origin of the product whose computation raised the failure, ORIGIN_NONE for one the operation raised outside every computation |

### presentation_dnit

```mach
pub fun presentation_dnit(p: *Presentation)
```

release a presentation, its store and its failure text; nil is a no-op

## QueryOutput

```mach
pub rec QueryOutput {
    bytes: *u8;
    len:   u32;
}
```

the bytes a derived product's computation publishes, allocated from the allocator it was handed

| Field | Type | |
| :--- | :--- | :--- |
| `bytes` | `*u8` | the product's bytes, nil exactly when `len` is 0 |
| `len` | `u32` | their length |

## QueryView

```mach
pub rec QueryView {
    value:        *u8;
    value_len:    u32;
    last_changed: Revision;
}
```

a borrowed view of a published product's bytes. valid only inside the operation that
produced it and until the next set_input, invalidate, retirement or teardown of the db;
the caller never frees it

| Field | Type | |
| :--- | :--- | :--- |
| `value` | `*u8` | the bytes the product published, owned by the db or the product's finalizer |
| `value_len` | `u32` | their length |
| `last_changed` | `Revision` | the revision the bytes last changed at |

## QueryDb

```mach
pub rec QueryDb {
    a:                    *std.allocator.Allocator;
    revision:             Revision;
    storage:              **QueryShard;
    storage_cap:          u32;
    kinds:                u32;
    compute_depth:        u32;
    stack:                Vector[StackFrame];
    prepared_count:       usize;
    operation_sequence:   u64;
    operation_active:     bool;
    presentation_pending: bool;
    rejections:           u64;
    attempts:             Vector[Attempt];
    attempt_index:        Map[Slot, usize];
    validated:            Map[Slot, Revision];
    # the attempt whose prepare is computing, which holds every failure raised meanwhile
    holding: opt[usize];
    # the failure the operation stands on, its text and places copied into and owned through `a`.
    # a failure the engine hands out borrows them until the operation's presentation is collected
    # or discarded, when the presentation takes the failure over or it is freed
    operation_failure: opt[mach.base.fail.Fail];
    # the origin of the product whose computation raised the standing failure, ORIGIN_NONE for
    # one raised outside every computation
    operation_origin: mach.base.diagnostic.Origin;
    # the failure the standing one displaced, owned like it and freed only with it, since a
    # computation may still read the copy it was handed. precedence keeps it to one: only a
    # defect displaces, and only a user's or the environment's failure
    operation_displaced: mach.base.fail.Fail;
    # the last operation's presentation, kept for its reader
    presentation: *Presentation;
    # the last held failure's text and places, owned through `a`, and the origin of the
    # product that raised it, ORIGIN_NONE for one no computation raised
    failure_message: str;
    failure_places:  mach.base.fail.Fail;
    failure_origin:  mach.base.diagnostic.Origin;
}
```

the query database: one shard per kind, the open operation's state and the failures it holds

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*std.allocator.Allocator` | owns the db, its shards and every derived product's bytes |
| `revision` | `Revision` | the latest revision issued |
| `storage` | `**QueryShard` | the shards, indexed by kind |
| `storage_cap` | `u32` | the capacity of `storage` |
| `kinds` | `u32` | how many shards `storage` holds |
| `compute_depth` | `u32` | how many computations are running |
| `stack` | `Vector[StackFrame]` | the products being computed or validated, innermost last |
| `prepared_count` | `usize` | how many prepared products are not yet completed or cancelled |
| `operation_sequence` | `u64` | the number of the latest operation |
| `operation_active` | `bool` | an operation is open |
| `presentation_pending` | `bool` | an ended operation's presentation is not yet collected or discarded |
| `rejections` | `u64` | how many reads in the open operation were rejected |
| `attempts` | `Vector[Attempt]` | each product the open operation tried, with its prepare and failure state |
| `attempt_index` | `Map[Slot, usize]` | each tried product's index in `attempts` |
| `validated` | `Map[Slot, Revision]` | each product the open operation read and the revision it read, 0 while it computes |

### init

```mach
pub fun init(a: *std.allocator.Allocator) res[QueryDb, mach.base.fail.Fail]
```

a database over a caller-owned allocator, holding no kind until a row is first used;
torn down with dnit, which runs every owned product's finalizer

| Parameter | |
| :--- | :--- |
| `a` | owns the db, its shards and every derived product's bytes |

### failure_dnit

```mach
pub fun failure_dnit(db: *QueryDb)
```

release the text and the places of the last failure the database holds

### adopt_failure

```mach
pub fun adopt_failure(db: *QueryDb, failure: mach.base.fail.Fail)
```

hold the failure an operation handed back, its text and places owned through the query
database, as the held failure, replacing what the slot held; the database owns it from here

### hold_failure

```mach
pub fun hold_failure(db: *QueryDb, failure: mach.base.fail.Fail, lost: *bool) mach.base.fail.Fail
```

hold a failure where the database presents it: its text and the places it points at are
copied into the database's slot, replacing what the slot held, and the failure returned reads
the held copies. text the presentation already owns stays there. a copy the allocator
refused leaves the failure without that part and sets `lost`

| Parameter | |
| :--- | :--- |
| `db` | the database |
| `failure` | the failure to hold; one with no text or places empties the slot |
| `lost` | set when part of the failure could not be held |
| **returns** | the failure as the database holds it |

### origin_of

```mach
pub fun origin_of(db: *QueryDb, f: mach.base.fail.Fail) mach.base.diagnostic.Origin
```

the origin of the product that raised `f`: the operation's, the held failure's or the last
presentation's when `f` reads that failure's text, ORIGIN_NONE for any other failure and for
one raised outside every computation

### presented_dnit

```mach
pub fun presented_dnit(db: *QueryDb)
```

release the last operation's presentation and the failure held beside it

### dnit

```mach
pub fun dnit(db: *QueryDb)
```

tear the database down: every owned product's finalizer runs on its bytes, every shard
and the operation state are released. nil is a no-op

### runtime

```mach
pub fun runtime[T](db: *QueryDb) res[QueryRuntime[T], mach.base.fail.Fail]
```

the runtime an operation's computations read the database through

| Parameter | |
| :--- | :--- |
| `db` | the database |
| **returns** | the runtime, or a failure when `db` is nil |

### begin

```mach
pub fun begin(db: *QueryDb) res[Operation, mach.base.fail.Fail]
```

open an operation. one is open at a time, and the previous one's presentation must
have been collected or discarded first; the validated set of the last operation is
cleared

| Parameter | |
| :--- | :--- |
| `db` | the database |
| **returns** | the handle every get, prepare and end of this operation takes |

### end

```mach
pub fun end(db: *QueryDb, operation: Operation) err[mach.base.fail.Fail]
```

close an operation: no computation may be on the stack and no prepared product may be
unfinished. the operation's presentation becomes pending and must be collected or
discarded before the next begin. err carries the failure the operation recorded; the
presentation is pending either way

| Parameter | |
| :--- | :--- |
| `db` | the database |
| `operation` | the handle begin returned |

### discard

```mach
pub fun discard(db: *QueryDb, operation: Operation) err[mach.base.fail.Fail]
```

release a pending presentation without collecting it: the attempts, the failure and
its message are dropped and the db accepts the next begin

| Parameter | |
| :--- | :--- |
| `db` | the database |
| `operation` | the ended operation's handle |

### take_failure

```mach
pub fun take_failure(db: *QueryDb, operation: Operation) res[opt[mach.base.fail.Fail], mach.base.fail.Fail]
```

take ownership of the ended operation's failure before discarding it: its text and places are
owned through the db's allocator and the caller frees them, and what borrowed them stays valid.
none when none was recorded

| Parameter | |
| :--- | :--- |
| `db` | the database |
| `operation` | the ended operation's handle |

### collect

```mach
pub fun collect(db: *QueryDb, operation: Operation, roots: *QueryKey, count: usize,
    a: *std.allocator.Allocator) res[*Presentation, mach.base.fail.Fail]
```

collect the ended operation's presentation: the diagnostics of every product reachable
from the roots through recorded dependencies, each once, plus the operation's failure,
then discard the pending state. on success the operation is over; on a
failure nothing was released and the presentation is still pending

| Parameter | |
| :--- | :--- |
| `db` | the database |
| `operation` | the ended operation's handle |
| `roots` | the products to walk from; nil only with count 0 |
| `count` | how many roots |
| `a` | owns the returned presentation |
| **returns** | the presentation, released with presentation_dnit, or the copy failure |

### set_input

```mach
pub fun set_input(db: *QueryDb, kind: *Kind, key: u64, value: *u8, value_len: u32) err[mach.base.fail.Fail]
```

set an input product's bytes, copied into the database. equal bytes keep the revision. an open
operation takes an input no computation in it has read, since nothing it holds observed the old
value; one it has read is refused

| Parameter | |
| :--- | :--- |
| `kind` | an INPUT row |

### prepare_retirement

```mach
pub fun prepare_retirement(db: *QueryDb, roots: *QueryKey, count: usize) res[PreparedRetirement, mach.base.fail.Fail]
```

a retirement rooted at the given products and every product that depends on them. a root
whose kind was never used has nothing to retire

| Parameter | |
| :--- | :--- |
| `db` | the database; no operation may be active |
| `roots` | the products to retire; nil only with count 0 |
| `count` | how many roots |
| **returns** | the prepared retirement, committed or discarded by the caller |

### prepare_owned_retirement

```mach
pub fun prepare_owned_retirement(db: *QueryDb, owned: OwnedFn, ctx: ptr) res[PreparedRetirement, mach.base.fail.Fail]
```

a retirement rooted at every product whose key a released module or file owns, with its dependents

| Parameter | |
| :--- | :--- |
| `db` | the database; no operation may be active |
| `owned` | whether a key's owner is released |
| `ctx` | owned's context |
| **returns** | the prepared retirement, committed or discarded by the caller |

### prepared_held

```mach
pub fun prepared_held(db: *QueryDb, prepared: PreparedQuery) res[bool, mach.base.fail.Fail]
```

whether the prepared query's compute failed, its failure held for complete_prepared to record

### preparing

```mach
pub fun preparing(db: *QueryDb, kind: *Kind, key: u64) bool
```

whether the computation on the stack is the prepare of the product `kind` keys under `key`, which a
compute reads to tell a prepare that finishes it outside the computation from a read that does not

### prepared_view

```mach
pub fun prepared_view(db: *QueryDb, prepared: PreparedQuery) res[QueryView, mach.base.fail.Fail]
```

a view of a prepared product's bytes, before they are published

| Parameter | |
| :--- | :--- |
| `db` | the database |
| `prepared` | the handle prepare returned |
| **returns** | the view, with last_changed 0, valid until the product is completed or cancelled, or a failure when the handle is stale or consumed |

### prepared_diags

```mach
pub fun prepared_diags(db: *QueryDb, prepared: PreparedQuery) res[*mach.base.diagnostic.DiagnosticStore, mach.base.fail.Fail]
```

the store a prepared query's compute reports into: work finished outside the
compute (a worker thread's optimize) appends there, so a refusal it records
reaches the presentation like one the compute made itself

### cancel_prepared

```mach
pub fun cancel_prepared(db: *QueryDb, prepared: PreparedQuery) err[mach.base.fail.Fail]
```

drop a prepared product unpublished, keeping its dependencies and diagnostics as the attempt's
evidence. a failure its compute raised stays unrecorded. a product with no earlier entry is
removed, and an earlier one is marked stale so it is not served

| Parameter | |
| :--- | :--- |
| `db` | the database |
| `prepared` | the handle prepare returned |
| **returns** | ok, or a failure when the handle is stale or consumed |

### complete_prepared

```mach
pub fun complete_prepared(db: *QueryDb, prepared: PreparedQuery, result: err[mach.base.fail.Fail]) err[mach.base.fail.Fail]
```

publish a prepared product, or record its failure: the one its compute raised, held until now, else
`result`, the failure of the work finished outside the computation. a failed product is dropped

### walk_products

```mach
pub fun walk_products(db: *QueryDb, root: QueryKey, ctx: ptr, enter: ProductFilter, visit: ProductVisit) err[mach.base.fail.Fail]
```

visit a product and every product it was computed from that the filter enters, each once
and its dependencies first, as presentation walks them. a product the filter refuses, or one
with no current value, ends its branch, so a walk costs what it enters, not the closure

| Parameter | |
| :--- | :--- |
| `db` | the database, its products as the open operation or the last one left them |
| `root` | the product to start from, itself subject to the filter |
| `ctx` | passed to enter and visit |
| `enter` | whether the walk enters a product |
| `visit` | shown each product entered |

### peek_revision

```mach
pub fun peek_revision(db: *QueryDb, kind: *Kind, key: u64) Revision
```

the revision a product last changed at, without validating it; 0 when it has none

### published

```mach
pub fun published(db: *QueryDb, kind: *Kind, key: u64, since: u64) opt[QueryView]
```

a product as its last successful computation published it, read between operations by the
host that ran them. none when no operation from `since` on validated or computed it, or when
its last validation or recomputation failed, so a failed product's previous value is never
served and a product the host's operations never reached is not taken for one they did

| Parameter | |
| :--- | :--- |
| `db` | the database; no operation may be active |
| `kind` | the product's row |
| `key` | the product's key |
| `since` | the first operation the host's reads count; an input is current whenever it was set |
| **returns** | a borrowed view, valid until the next operation, input change or retirement |

### value_dnit

```mach
pub fun value_dnit(db: *QueryDb, kind: *Kind, key: u64) err[mach.base.fail.Fail]
```

give back the value of a derived product no reader of the build reads again: its finalizer
runs and its bytes go, while the entry stays current, so a product computed from it still
validates against its revision. a later read of it is a failure, and a recomputation, once
what it read moves, serves a new value

| Parameter | |
| :--- | :--- |
| `db` | the database; the product is the owner thread's |
| `kind` | the product's row, a derived one |
| `key` | the product's key within the kind |
| **returns** | ok, also when the product was never computed or is released already |

### views_add

```mach
pub fun views_add(db: *QueryDb, v: *Views, kind: *Kind) err[mach.base.fail.Fail]
```

every product of `kind` validated in this operation added to `v`

### views_add_deps

```mach
pub fun views_add_deps(db: *QueryDb, v: *Views, of: *Kind, key: u64, kind: *Kind) err[mach.base.fail.Fail]
```

every product of `kind` the product `of` keyed `key` read, validated in this operation, added to `v`

### replay

```mach
pub fun replay(db: *QueryDb, log: *ReadLog) err[mach.base.fail.Fail]
```

the reads `log` holds past `done` replayed onto the computing frame, as `get` records a read of a
validated product: the edge, then its revisions. a product changed since its read is a defect

## QueryRuntime

```mach
pub rec QueryRuntime[T] {
    db: *QueryDb;
}
```

the database as an operation's computations see it: each product is computed with the
runtime's context, a `T` the computations of every kind read through

### get

```mach
pub fun get[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) res[QueryView, mach.base.fail.Fail]
```

the current value of a product, computing or revalidating it inside the open operation
and recording it as a dependency of the product being computed, if any

| Parameter | |
| :--- | :--- |
| `rt` | the runtime of the open operation |
| `ctx` | the context every row's computation reads through |
| `kind` | the product's row |
| `key` | the product's key within the kind |
| **returns** | a borrowed view (QueryView's lifetime), or the product's failure |

### reusable

```mach
pub fun reusable[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) res[bool, mach.base.fail.Fail]
```

validation may refresh dependencies but never computes the requested product

### prepare

```mach
pub fun prepare[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) res[PreparedQuery, mach.base.fail.Fail]
```

compute a derived product the caller finishes outside the computation (a worker thread),
published by complete_prepared or dropped by cancel_prepared before the operation ends. a
failure raised while it computes is the prepared product's: it is held, and recorded by
complete_prepared or left unrecorded by cancel_prepared, so a product prepared ahead never
fails the operation before the products the caller completes ahead of it

### revision

```mach
pub fun revision[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) res[Revision, mach.base.fail.Fail]
```

the revision a product last changed at, read as `get` reads it

| Parameter | |
| :--- | :--- |
| `rt` | the runtime of the open operation |
| `ctx` | the context every row's computation reads through |
| `kind` | the product's row |
| `key` | the product's key within the kind |
| **returns** | the revision, or the product's failure |

### depend

```mach
pub fun depend[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) err[mach.base.fail.Fail]
```

record a dependency on a product without reading its bytes, validating or computing it as
`get` does

| Parameter | |
| :--- | :--- |
| `rt` | the runtime of the open operation |
| `ctx` | the context every row's computation reads through |
| `kind` | the product's row |
| `key` | the product's key within the kind |
| **returns** | ok, or the product's failure |

## PreparedRetirement

```mach
pub rec PreparedRetirement {
    owner:     *QueryDb;
    operation: u64;
    revision:  Revision;
    keys:      Vector[Slot];
    selected:  Map[Slot, usize];
}
```

the products a retirement removes, valid while the database stays as it was prepared against

| Field | Type | |
| :--- | :--- | :--- |
| `owner` | `*QueryDb` | the database, nil once committed or discarded |
| `operation` | `u64` | the database's operation number when prepared |
| `revision` | `Revision` | the database's revision when prepared |
| `keys` | `Vector[Slot]` | the selected products, each before the products it read |
| `selected` | `Map[Slot, usize]` | every selected product |

### discard_retirement

```mach
pub fun discard_retirement(prepared: *PreparedRetirement)
```

release a retirement that was prepared and not committed; one already committed or
discarded owns nothing

### validate_retirement

```mach
pub fun validate_retirement(prepared: *PreparedRetirement) err[mach.base.fail.Fail]
```

whether a prepared retirement still matches its database

| Parameter | |
| :--- | :--- |
| `prepared` | the prepared retirement |
| **returns** | ok, or a failure when it was committed or discarded, an operation is active or pending, or the database ran an operation or changed revision since it was prepared |

### commit_retirement

```mach
pub fun commit_retirement(prepared: *PreparedRetirement) err[mach.base.fail.Fail]
```

remove every selected product, its finalizer run on its bytes, and release the retirement
a retirement that removes anything takes a new revision

| Parameter | |
| :--- | :--- |
| `prepared` | the prepared retirement, released on success |
| **returns** | ok, or the failure validate_retirement reports, leaving the retirement prepared |

## ProductFilter

```mach
pub def ProductFilter: fun(ptr, QueryKey) bool
```

whether a product walk enters a product, visiting it and walking what it was computed from
reached through walk_products

## ProductVisit

```mach
pub def ProductVisit: fun(ptr, QueryKey, *mach.base.diagnostic.DiagnosticStore)
```

what a product walk is shown of each product: its address and the store it reported into,
nil when it reported nothing
reached through walk_products

## Read

```mach
pub rec Read {
    kind:                KindIndex;
    key:                 u64;
    last_changed:        Revision;
    diagnostics_changed: Revision;
}
```

a computation that reads off the stack. it reads only products validated in this operation before
it started, through views the owner gathered, and logs each read. the owner replays the log onto
the computing frame in log order, so the edges are those the same reads made through `get` would
have recorded, at the same place in the list. views and the log touch no map or shard of the
database, and no view is added once the reading starts, so the reading may run on another thread
while the owner works on. a read the views do not hold is a miss, which the reader answers on the
owner (replay what was logged, then `get`) or by giving the reading up

## ReadLog

```mach
pub rec ReadLog {
    reads: Vector[Read];
    done:  usize;
}
```

the reads of one computation in the order it made them, those before `done` replayed

### log_init

```mach
pub fun log_init(a: *std.allocator.Allocator) ReadLog
```

an empty read log

| Parameter | |
| :--- | :--- |
| `a` | the allocator its reads draw from |
| **returns** | the log, released with `log_dnit` |

### log_dnit

```mach
pub fun log_dnit(log: *ReadLog)
```

release a read log

## ReadFn

```mach
pub def ReadFn: fun(ptr, *Kind, u64) res[QueryView, mach.base.fail.Fail]
```

how a computation reads a product: through `get` on the owner, or through views it logs
reached through Reads.read

## Reads

```mach
pub rec Reads {
    ctx:  ptr;
    read: ReadFn;
}
```

how one computation reads products

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | passed to `read` |
| `read` | `ReadFn` | reads one product |

## Views

```mach
pub rec Views {
    base:    *Views;
    rows:    Vector[ViewRow];
    entries: Map[Slot, *QueryEntry];
}
```

products a reading may read, each validated in this operation, ready and not given back. a view
set may sit over a `base` set, which it is read through after its own

### views_init

```mach
pub fun views_init(a: *std.allocator.Allocator, base: *Views) Views
```

an empty view set

| Parameter | |
| :--- | :--- |
| `a` | the allocator its rows and entries draw from |
| `base` | the set read after this one, nil for none, borrowed for the set's life |
| **returns** | the set, released with `views_dnit` |

### views_dnit

```mach
pub fun views_dnit(v: *Views)
```

release a view set, leaving its base alone

### view_read

```mach
pub fun view_read(v: *Views, log: *ReadLog, kind: *Kind, key: u64) res[opt[QueryView], mach.base.fail.Fail]
```

the product `kind` keys under `key` read through `v` and logged, or none when `v` does not hold it

