# mach.lang.driver.demand

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

## val UNIT_BUCKETS

```mach
pub val UNIT_BUCKETS: u32 = 8
```

how many units the instances of one generic's module spread over

## val SUFFIX_CAP

```mach
pub val SUFFIX_CAP: usize = 24
```

the bytes a unit's suffix needs at most

## rec Registry

```mach
pub rec Registry;
```

every instance a build has met, by its symbol: its identity as whoever asked for it last
described it, which the instance's typing reads. it lives as long as the project, since an
instance is typed again in a later operation of the build when what it reads moved

## fun registry_init

```mach
pub fun registry_init(a: *A.Allocator) Registry;
```

a registry holding nothing yet: its set is made on the first instance it keeps

## fun registry_dnit

```mach
pub fun registry_dnit(r: *Registry);
```

## fun register

```mach
pub fun register(r: *Registry, s: *session.Session, item: *sema_instance.Instance) res[intern.StrId, fail.Fail];
```

the symbol of the instance `item` describes, its description kept under it

## fun described

```mach
pub fun described(r: *Registry, name: intern.StrId) opt[*sema_instance.Instance];
```

the instance the symbol `name` names, none when nothing described it

## rec Typer

```mach
pub rec Typer;
```

how the demand types an instance, and has a module typed for an instance only that module's
record names

typed: the typing of the instance a symbol and its description name
asked: the instance set of a module whose record stood in for its typing, its front end run for
       it, nil where no record stands in for a module

## rec Member

```mach
pub rec Member;
```

one demanded instance

name: its symbol
text: the symbol's text, which orders a unit's members
origin: the module that declares the generic
fqn: the origin's name, which orders units
stable: the origin's stable id
bucket: the unit of the origin it falls in

## rec Unit

```mach
pub rec Unit;
```

one instance unit: its query key and the run of members it holds

## rec Entry

```mach
pub rec Entry;
```

one instance the closure reached

origin: the module that declares the generic, nil until its description or its record names it
typed: its typing this build, nil where its record stood in for it or it could not be typed;
        held only while the closure is walked
record: its record in the demand's records when that stood in for its typing, NONE otherwise
reach: the objects that reach it, the least fixed point over every edge into it
asker: the first module whose record asks for it, nil when none does
parent: the first entry whose body asks for it, NONE when none does

## rec Demand

```mach
pub rec Demand;
```

the closure while it is walked, and its members grouped into units once settled. the entries
stay with the demand, so an instance a unit must lower can be described after it

rejected: an instance in the closure does not type check, or could not be typed
records: the instance records this build reads, nil for none (borrowed)
fresh: a record of each instance this build typed whose typing can be recorded, made by the
          demand's owner

## fun init

```mach
pub fun init(a: *A.Allocator) Demand;
```

## fun dnit

```mach
pub fun dnit(d: *Demand);
```

## fun unit_key

```mach
pub fun unit_key(stable: module.StableModuleId, bucket: u32) u64;
```

a unit's query key: its origin's stable id in the low bits, its bucket above them

## fun key_stable

```mach
pub fun key_stable(key: u64) module.StableModuleId;
```

## fun key_bucket

```mach
pub fun key_bucket(key: u64) u32;
```

## fun bucket_of

```mach
pub fun bucket_of(text: str) u32;
```

the bucket of an instance's symbol: fnv-1a over its text, fixed across builds and compilers

## fun unit_suffix

```mach
pub fun unit_suffix(bucket: u32, buf: *u8) str;
```

what a unit is called after its origin's name, `instances.<bucket>`, written into `buf`, which
holds SUFFIX_CAP bytes

## fun offer_module

```mach
pub fun offer_module(d: *Demand, r: *Registry, s: *session.Session, set: *sema_instance.Set, emitted: bool) err[fail.Fail];
```

every instance the module's `set` asks for, each reaching what the module's own code reaches it
from when `emitted`, the build compiling the module, and nothing otherwise: a module the build
only types still has its instances typed

## fun offer_recorded

```mach
pub fun offer_recorded(d: *Demand, s: *session.Session, mid: module.ModuleId, file: lang_source.FileId, asked: *cache_record.Asked, count: u32, emitted: bool) err[fail.Fail];
```

every instance the record of module `mid`, whose source is `file`, says its code asks for, as
offer_module offers a typed module's: the record stands in for the module's typing, and the
module is typed only when an instance it asks for must be described

## fun close

```mach
pub fun close(d: *Demand, r: *Registry, s: *session.Session, t: Typer) err[fail.Fail];
```

the closure: every entry met, in the order met, its demand read from the record that stands in
for its typing or else from its typing, and every instance it asks for met in turn. an instance
that could not be typed, its declaring module rejected, asks for nothing more. then places: an
instance with reports whose record's places no longer stand is typed, so its reports point into
its body where the body now is

## fun describe_named

```mach
pub fun describe_named(d: *Demand, r: *Registry, s: *session.Session, t: Typer, name: intern.StrId) err[fail.Fail];
```

the instance `name` of a settled demand described in the registry, for a unit that must lower it

## fun recorded

```mach
pub fun recorded(d: *Demand, name: intern.StrId) u32;
```

the record that stood in for the typing of the instance `name`, NONE when it was typed

## fun report

```mach
pub fun report(d: *Demand, s: *session.Session, sources: *diagnostic.SourceTable, into: *diagnostic.DiagnosticStore) err[fail.Fail];
```

each instance's reports, from its typing or its record. a report its body places itself goes
out once, in the order of the instances' symbols. a report placed nowhere waits for a requester:
every module whose own code reaches the instance, directly or through other instances, reports
it once, at its first call site by offset whose closure reaches it. modules go by their file's
path and, at one site, instances by their symbols, so the order is the sources' alone. a recorded
report reads its places through `sources`, which may be nil when no record stood in

## fun settle

```mach
pub fun settle(d: *Demand, s: *session.Session, wanted: sema_instance.Reach) err[fail.Fail];
```

reach carried along every edge to its least fixed point, then the instances that reach meets
`wanted` in, ordered by origin name, bucket and symbol and grouped into units. the typing the
walk held is the operation's, and no later reader holds it

## fun unit_of

```mach
pub fun unit_of(d: *Demand, key: u64) opt[*Unit];
```

the unit keyed `key`, none when the demand holds no instance of it

## fun same

```mach
pub fun same(a: *Demand, b: *Demand) bool;
```

whether two settled demands hold the same members in the same units

