# mach.lang.query

the query database: every product is of one kind, a `Kind` row its owner
declares beside the computation. a row says how its bytes are owned (an input
the host sets, derived bytes the database owns, an owned handle released by its
finalizer, or a metadata revision sampled from outside), what its keys name and
how a derived product is computed; the database takes a row on its first use and
knows no kind of its own. a product records its dependencies by reading them
inside its compute (`get`, `depend`, `observe`), and owns the diagnostic store
its compute wrote into. validation is transitive and depth first; equal bytes
on recomputation keep the previous revision and stop propagation; a failed
dependency fails the product and the previous value is never served. all reads
happen inside one operation, and `end` refuses success when a sampled metadata
revision moved

## def KeyOwner

```mach
pub def KeyOwner: u8
```

what a kind's keys name, so a session can release every product a module or file left behind.
a kind keyed by a module carries its stable module id in the key's low 32 bits, whatever the
higher bits add (a test build), and a kind keyed by a file is keyed by its file id alone. a row
left at KEY_UNDECLARED is refused on its first use, so a kind cannot leak by omission

## val KEY_UNOWNED

```mach
pub val KEY_UNOWNED:    KeyOwner = 1
```

## val KEY_MODULE

```mach
pub val KEY_MODULE:     KeyOwner = 2
```

## val KEY_FILE

```mach
pub val KEY_FILE:       KeyOwner = 3
```

## def Revision

```mach
pub def Revision: u64
```

## def Role

```mach
pub def Role: u8
```

how a kind's products come to be: set by the host, computed, or sampled from outside
reached through Kind.role

## val INPUT

```mach
pub val INPUT:           Role = 1
```

## val DERIVED

```mach
pub val DERIVED:         Role = 2
```

## def FinalizeFn

```mach
pub def FinalizeFn: fun(*u8, u32, *A.Allocator)
```

releases what an owned product's bytes hold
reached through Kind.finalize

## def EqualFn

```mach
pub def EqualFn: fun(*u8, u32, *u8, u32) bool
```

whether two owned products are the same
reached through Kind.equal

## def ComputeFn

```mach
pub def ComputeFn: fun(ptr, *Kind, u64, *A.Allocator, *diagnostic.DiagnosticStore) res[QueryOutput, fail.Fail]
```

a derived product's computation: the context the operation's runtime carries, the row it
computes, the key, the allocator its bytes come from and the store it reports into
reached through Kind.compute

## rec Kind

```mach
pub rec Kind;
```

one kind of product, declared once beside the computation that owns it. the database takes
the row on its first use; a row's address is its identity, so two rows never share a shard

name: how failures and cycles name the kind
role: INPUT or DERIVED
owner: what the kind's keys name
origin: the phase a derived product's diagnostics belong to
compute: a derived product's computation; an input has none
finalize: releases what an owned derived product's bytes hold, called only on bytes its own
          computation published; nil when the bytes own nothing
equal: whether two owned products are the same, so an equal recomputation keeps the revision;
          requires finalize
replay: the product lowers or generates a module's object, so a restored object replays its
          warnings even when the module's front end ran again and reported its own

## rec QueryKey

```mach
pub rec QueryKey;
```

a product's address: the row of its kind and its key

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

## rec QueryDb

```mach
pub rec QueryDb;
```

## rec QueryRuntime

```mach
pub rec QueryRuntime[T];
```

the database as an operation's computations see it: each product is computed with the
runtime's context, a `T` the computations of every kind read through

## fun init

```mach
pub fun init(a: *A.Allocator) res[QueryDb, fail.Fail];
```

a database over a caller-owned allocator, holding no kind until a row is first used;
torn down with dnit, which runs every owned product's finalizer

a: owns the db, its shards and every derived product's bytes

## fun drop_failure

```mach
pub fun drop_failure(db: *QueryDb);
```

release the text and the places of the last failure the database holds

## fun adopt_failure_text

```mach
pub fun adopt_failure_text(db: *QueryDb, text: str);
```

hold the text an operation handed back, allocated from the query database, as the held
failure's text; the database owns it from here

## fun hold_failure

```mach
pub fun hold_failure(db: *QueryDb, failure: fail.Fail, lost: *bool) fail.Fail;
```

hold a failure where the database presents it: its text and the places it points at are
copied into the database's slot, replacing what the slot held, and the failure returned reads
the held copies. text the presentation already owns stays there. a copy the allocator
refused leaves the failure without that part and sets `lost`

db: the database
failure: the failure to hold; one with no text or places empties the slot
lost: set when part of the failure could not be held
ret: the failure as the database holds it

## fun drop_presented

```mach
pub fun drop_presented(db: *QueryDb);
```

release the last operation's presentation and the failure held beside it

## fun dnit

```mach
pub fun dnit(db: *QueryDb);
```

tear the database down: every owned product's finalizer runs on its bytes, every shard
and the operation state are released. nil is a no-op

## fun runtime

```mach
pub fun runtime[T](db: *QueryDb) res[QueryRuntime[T], fail.Fail];
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

## fun set_input

```mach
pub fun set_input(db: *QueryDb, kind: *Kind, key: u64, value: *u8, value_len: u32) err[fail.Fail];
```

set an input product's bytes, copied into the database. equal bytes keep the revision

kind: an INPUT row

## rec PreparedRetirement

```mach
pub rec PreparedRetirement;
```

## fun discard_retirement

```mach
pub fun discard_retirement(prepared: *PreparedRetirement);
```

release a retirement that was prepared and not committed; one already committed or
discarded owns nothing

## fun prepare_retirement

```mach
pub fun prepare_retirement(db: *QueryDb, roots: *QueryKey, count: usize) res[PreparedRetirement, fail.Fail];
```

a retirement rooted at the given products and every product that depends on them. a root
whose kind was never used has nothing to retire

db: the database; no operation may be active
roots: the products to retire; nil only with count 0
count: how many roots
ret: the prepared retirement, committed or discarded by the caller

## fun prepare_owned_retirement

```mach
pub fun prepare_owned_retirement(db: *QueryDb, owned: OwnedFn, ctx: ptr) res[PreparedRetirement, fail.Fail];
```

a retirement rooted at every product whose key a released module or file owns, with its dependents

db: the database; no operation may be active
owned: whether a key's owner is released
ctx: owned's context
ret: the prepared retirement, committed or discarded by the caller

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
pub fun get[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) res[QueryView, fail.Fail];
```

the current value of a product, computing or revalidating it inside the open operation
and recording it as a dependency of the product being computed, if any

rt: the runtime of the open operation
ctx: the context every row's computation reads through
kind: the product's row
key: the product's key within the kind
ret: a borrowed view (QueryView's lifetime), or the product's failure

## fun reusable

```mach
pub fun reusable[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) res[bool, fail.Fail];
```

validation may refresh dependencies but never computes the requested product

## fun prepare

```mach
pub fun prepare[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) res[PreparedQuery, fail.Fail];
```

compute a derived product the caller finishes outside the computation (a worker thread),
published by complete_prepared or dropped by cancel_prepared before the operation ends

## fun prepared_view

```mach
pub fun prepared_view(db: *QueryDb, prepared: PreparedQuery) res[QueryView, fail.Fail];
```

## fun prepared_diags

```mach
pub fun prepared_diags(db: *QueryDb, prepared: PreparedQuery) res[*diagnostic.DiagnosticStore, fail.Fail];
```

the store a prepared query's compute reports into: work finished outside the
compute (a worker thread's optimize) appends there, so a refusal it records
reaches the presentation like one the compute made itself

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
pub fun revision[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) res[Revision, fail.Fail];
```

## fun depend

```mach
pub fun depend[T](rt: *QueryRuntime[T], ctx: *T, kind: *Kind, key: u64) err[fail.Fail];
```

## def ProductFilter

```mach
pub def ProductFilter: fun(ptr, QueryKey) bool
```

whether a product walk enters a product, visiting it and walking what it was computed from
reached through walk_products

## def ProductVisit

```mach
pub def ProductVisit: fun(ptr, QueryKey, *diagnostic.DiagnosticStore)
```

what a product walk is shown of each product: its address and the store it reported into,
nil when it reported nothing
reached through walk_products

## fun walk_products

```mach
pub fun walk_products(db: *QueryDb, root: QueryKey, ctx: ptr, enter: ProductFilter, visit: ProductVisit) err[fail.Fail];
```

visit a product and every product it was computed from that the filter enters, each once
and its dependencies first, as presentation walks them. a product the filter refuses, or one
with no current value, ends its branch, so a walk costs what it enters, not the closure

db: the database, its products as the open operation or the last one left them
root: the product to start from, itself subject to the filter
ctx: passed to enter and visit
enter: whether the walk enters a product
visit: shown each product entered

## fun peek_revision

```mach
pub fun peek_revision(db: *QueryDb, kind: *Kind, key: u64) Revision;
```

the revision a product last changed at, without validating it; 0 when it has none

## fun published

```mach
pub fun published(db: *QueryDb, kind: *Kind, key: u64, since: u64) opt[QueryView];
```

a product as its last successful computation published it, read between operations by the
host that ran them. none when no operation from `since` on validated or computed it, or when
its last validation or recomputation failed, so a failed product's previous value is never
served and a product the host's operations never reached is not taken for one they did

db: the database; no operation may be active
kind: the product's row
key: the product's key
since: the first operation the host's reads count; an input is current whenever it was set
ret: a borrowed view, valid until the next operation, input change or retirement

## fun release_value

```mach
pub fun release_value(db: *QueryDb, kind: *Kind, key: u64) err[fail.Fail];
```

give back the value of a derived product no reader of the build reads again: its finalizer
runs and its bytes go, while the entry stays current, so a product computed from it still
validates against its revision. a later read of it is a failure, and a recomputation, once
what it read moves, serves a new value

db: the database; the product is the owner thread's
kind: the product's row, a derived one
key: the product's key within the kind
ret: ok, also when the product was never computed or is released already

