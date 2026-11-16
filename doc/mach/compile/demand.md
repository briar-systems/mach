[mach](../README.md) / [compile](README.md) / **demand**

# mach.compile.demand

the build's demand for generic instances and the instance units it falls into. every module
asks for the instances its own code calls, and every instance for those its body calls; the
build's demand is the closure of the first through the second, each instance typed once, by
itself. a module whose code reaches an instance only declares it: the instance is lowered and
compiled once per build, in the unit its identity alone names, the module declaring the
generic and a fixed hash of the instance's mangled name modulo UNIT_BUCKETS. adding or
dropping one instance changes exactly one unit, and nothing the demand holds depends on the
order modules were loaded, typed or compiled in: the closure of a set is the same however it
is walked, reach is its least fixed point, members are ordered by mangled name, units by
their origin's name and bucket, a report goes to every module whose code reaches its instance,
at that module's first call site leading there, and no dense id reaches a name. a module whose front end the build skips offers what its
cache record lists, and an instance whose record stands is met through what the record lists,
typed only when it must place a report its declaring file moved

<sub>source: [`src/compile/demand.mach`](../../../src/compile/demand.mach) · imported by 4 modules</sub>

<sub>`sema_instance` is [`mach.front.sema.instance`](../front/sema/instance.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `A` is `std.allocator` · `session` is [`mach.session`](../session.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `sema_product` is [`mach.front.sema.product`](../front/sema/product.md) · `module` is [`mach.session.module`](../session/module.md) · `Edge` is private · `Ask` is private · `cache_instances` is [`mach.compile.cache.instances`](cache/instances.md) · `base_source` is [`mach.base.source`](../base/source.md) · `cache_record` is [`mach.compile.cache.record`](cache/record.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Registry`](#registry) | `rec` | every instance a build has met, by its symbol: its identity as whoever asked for it last described it, which the instance's typing reads |
| [`Typer`](#typer) | `rec` | how the demand types an instance, and has a module typed for an instance only that module's record names |
| [`Member`](#member) | `rec` | one demanded instance |
| [`Unit`](#unit) | `rec` | one instance unit: its query key and the run of members it holds |
| [`Entry`](#entry) | `rec` | one instance the closure reached |
| [`Demand`](#demand) | `rec` | the closure while it is walked, and its members grouped into units once settled |

| On `Registry` | |
| :--- | :--- |
| [`registry_init`](#registry_init) | a registry holding nothing yet: its set is made on the first instance it keeps |
| [`registry_dnit`](#registry_dnit) | |
| [`register`](#register) | the symbol of the instance `item` describes, its description kept under it |
| [`described`](#described) | the instance the symbol `name` names, none when nothing described it |

| On `Demand` | |
| :--- | :--- |
| [`init`](#init) · [`dnit`](#dnit) | |
| [`offer_module`](#offer_module) | every instance the module's `set` asks for, each reaching what the module's own code reaches it from when `emitted`, the build compiling the module, and nothing otherwise: a module the build only types still has its instances typed |
| [`offer_recorded`](#offer_recorded) | every instance the record of module `mid`, whose source is `file`, says its code asks for, as offer_module offers a typed module's: the record stands in for the module's typing, and the module is typed only when an instance it asks for must be described |
| [`close`](#close) | the closure: every entry met, in the order met, its demand read from the record that stands in for its typing or else from its typing, and every instance it asks for met in turn |
| [`describe_named`](#describe_named) | the instance `name` of a settled demand described in the registry, for a unit that must lower it |
| [`recorded`](#recorded) | the record that stood in for the typing of the instance `name`, NONE when it was typed |
| [`report`](#report) | each instance's reports, from its typing or its record |
| [`settle`](#settle) | reach carried along every edge to its least fixed point, then the instances that reach meets `wanted` in, ordered by origin name, bucket and symbol and grouped into units |
| [`unit_of`](#unit_of) | the unit keyed `key`, none when the demand holds no instance of it |
| [`same`](#same) | whether two settled demands hold the same members in the same units |

| Functions | |
| :--- | :--- |
| [`unit_key`](#unit_key) | a unit's query key: its origin's stable id in the low bits, its bucket above them |
| [`key_stable`](#key_stable) · [`key_bucket`](#key_bucket) | |
| [`bucket_of`](#bucket_of) | the bucket of an instance's symbol: fnv-1a over its text, fixed across builds and compilers |
| [`unit_suffix`](#unit_suffix) | what a unit is called after its origin's name, `instances.<bucket>`, written into `buf`, which holds SUFFIX_CAP bytes |

| Constants | |
| :--- | :--- |
| [`UNIT_BUCKETS`](#constants) | how many units the instances of one generic's module spread over |
| [`SUFFIX_CAP`](#constants) | the bytes a unit's suffix needs at most |

---

## Registry

```mach
pub rec Registry {
    set:   sema_instance.Set;
    names: Map[intern.StrId, u32];
}
```

every instance a build has met, by its symbol: its identity as whoever asked for it last
described it, which the instance's typing reads. it lives as long as the project, since an
instance is typed again in a later operation of the build when what it reads moved

### registry_init

```mach
pub fun registry_init(a: *A.Allocator) Registry
```

a registry holding nothing yet: its set is made on the first instance it keeps

### registry_dnit

```mach
pub fun registry_dnit(r: *Registry)
```

### register

```mach
pub fun register(r: *Registry, s: *session.Session, item: *sema_instance.Instance) res[intern.StrId, fail.Fail]
```

the symbol of the instance `item` describes, its description kept under it

### described

```mach
pub fun described(r: *Registry, name: intern.StrId) opt[*sema_instance.Instance]
```

the instance the symbol `name` names, none when nothing described it

## Typer

```mach
pub rec Typer {
    ctx:   ptr;
    typed: fun(ptr, intern.StrId, *sema_instance.Instance) res[*sema_product.InstanceResult, fail.Fail];
    asked: fun(ptr, module.ModuleId) res[*sema_instance.Set, fail.Fail];
}
```

how the demand types an instance, and has a module typed for an instance only that module's
record names

| Field | Type | |
| :--- | :--- | :--- |
| `typed` | `fun(ptr, intern.StrId, *sema_instance.Instance) res[*sema_product.InstanceResult, fail.Fail]` | the typing of the instance a symbol and its description name |
| `asked` | `fun(ptr, module.ModuleId) res[*sema_instance.Set, fail.Fail]` | the instance set of a module whose record stood in for its typing, its front end run for it, nil where no record stands in for a module |

## Member

```mach
pub rec Member {
    name:   intern.StrId;
    text:   str;
    origin: module.ModuleId;
    fqn:    str;
    stable: module.StableModuleId;
    bucket: u32;
}
```

one demanded instance

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | its symbol |
| `text` | `str` | the symbol's text, which orders a unit's members |
| `origin` | `module.ModuleId` | the module that declares the generic |
| `fqn` | `str` | the origin's name, which orders units |
| `stable` | `module.StableModuleId` | the origin's stable id |
| `bucket` | `u32` | the unit of the origin it falls in |

## Unit

```mach
pub rec Unit {
    key:    u64;
    origin: module.ModuleId;
    bucket: u32;
    first:  u32;
    count:  u32;
}
```

one instance unit: its query key and the run of members it holds

## Entry

```mach
pub rec Entry {
    name:   intern.StrId;
    text:   str;
    origin: module.ModuleId;
    typed:  *sema_product.InstanceResult;
    record: u32;
    reach:  sema_instance.Reach;
    asker:  module.ModuleId;
    parent: u32;
}
```

one instance the closure reached

| Field | Type | |
| :--- | :--- | :--- |
| `origin` | `module.ModuleId` | the module that declares the generic, nil until its description or its record names it |
| `typed` | `*sema_product.InstanceResult` | its typing this build, nil where its record stood in for it or it could not be typed; held only while the closure is walked |
| `record` | `u32` | its record in the demand's records when that stood in for its typing, NONE otherwise |
| `reach` | `sema_instance.Reach` | the objects that reach it, the least fixed point over every edge into it |
| `asker` | `module.ModuleId` | the first module whose record asks for it, nil when none does |
| `parent` | `u32` | the first entry whose body asks for it, NONE when none does |

## Demand

```mach
pub rec Demand {
    members:  Vector[Member];
    units:    Vector[Unit];
    entries:  Vector[Entry];
    edges:    Vector[Edge];
    asks:     Vector[Ask];
    index:    Map[intern.StrId, u32];
    rejected: bool;
    records:  *cache_instances.Store;
    fresh:    cache_instances.Store;
}
```

the closure while it is walked, and its members grouped into units once settled. the entries
stay with the demand, so an instance a unit must lower can be described after it

| Field | Type | |
| :--- | :--- | :--- |
| `rejected` | `bool` | an instance in the closure does not type check, or could not be typed |
| `records` | `*cache_instances.Store` | the instance records this build reads, nil for none (borrowed) |
| `fresh` | `cache_instances.Store` | a record of each instance this build typed whose typing can be recorded, made by the demand's owner |

### init

```mach
pub fun init(a: *A.Allocator) Demand
```

### dnit

```mach
pub fun dnit(d: *Demand)
```

### offer_module

```mach
pub fun offer_module(d: *Demand, r: *Registry, s: *session.Session, set: *sema_instance.Set, emitted: bool) err[fail.Fail]
```

every instance the module's `set` asks for, each reaching what the module's own code reaches it
from when `emitted`, the build compiling the module, and nothing otherwise: a module the build
only types still has its instances typed

### offer_recorded

```mach
pub fun offer_recorded(d: *Demand, s: *session.Session, mid: module.ModuleId, file: base_source.FileId, asked: *cache_record.Asked, count: u32, emitted: bool) err[fail.Fail]
```

every instance the record of module `mid`, whose source is `file`, says its code asks for, as
offer_module offers a typed module's: the record stands in for the module's typing, and the
module is typed only when an instance it asks for must be described

### close

```mach
pub fun close(d: *Demand, r: *Registry, s: *session.Session, t: Typer) err[fail.Fail]
```

the closure: every entry met, in the order met, its demand read from the record that stands in
for its typing or else from its typing, and every instance it asks for met in turn. an instance
that could not be typed, its declaring module rejected, asks for nothing more. then places: an
instance with reports whose record's places no longer stand is typed, so its reports point into
its body where the body now is

### describe_named

```mach
pub fun describe_named(d: *Demand, r: *Registry, s: *session.Session, t: Typer, name: intern.StrId) err[fail.Fail]
```

the instance `name` of a settled demand described in the registry, for a unit that must lower it

### recorded

```mach
pub fun recorded(d: *Demand, name: intern.StrId) u32
```

the record that stood in for the typing of the instance `name`, NONE when it was typed

### report

```mach
pub fun report(d: *Demand, s: *session.Session, sources: *diagnostic.SourceTable, into: *diagnostic.DiagnosticStore) err[fail.Fail]
```

each instance's reports, from its typing or its record. a report its body places itself goes
out once, in the order of the instances' symbols. a report placed nowhere waits for a requester:
every module whose own code reaches the instance, directly or through other instances, reports
it once, at its first call site by offset whose closure reaches it. modules go by their file's
path and, at one site, instances by their symbols, so the order is the sources' alone. a recorded
report reads its places through `sources`, which may be nil when no record stood in

### settle

```mach
pub fun settle(d: *Demand, s: *session.Session, wanted: sema_instance.Reach) err[fail.Fail]
```

reach carried along every edge to its least fixed point, then the instances that reach meets
`wanted` in, ordered by origin name, bucket and symbol and grouped into units. the typing the
walk held is the operation's, and no later reader holds it

### unit_of

```mach
pub fun unit_of(d: *Demand, key: u64) opt[*Unit]
```

the unit keyed `key`, none when the demand holds no instance of it

### same

```mach
pub fun same(a: *Demand, b: *Demand) bool
```

whether two settled demands hold the same members in the same units

## Functions

### unit_key

```mach
pub fun unit_key(stable: module.StableModuleId, bucket: u32) u64
```

a unit's query key: its origin's stable id in the low bits, its bucket above them

### key_stable

```mach
pub fun key_stable(key: u64) module.StableModuleId
```

### key_bucket

```mach
pub fun key_bucket(key: u64) u32
```

### bucket_of

```mach
pub fun bucket_of(text: str) u32
```

the bucket of an instance's symbol: fnv-1a over its text, fixed across builds and compilers

### unit_suffix

```mach
pub fun unit_suffix(bucket: u32, buf: *u8) str
```

what a unit is called after its origin's name, `instances.<bucket>`, written into `buf`, which
holds SUFFIX_CAP bytes

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `UNIT_BUCKETS` | `u32` | `8` | how many units the instances of one generic's module spread over |
| `SUFFIX_CAP` | `usize` | `24` | the bytes a unit's suffix needs at most |

