# mach.lang.driver.cache.instances

the instance records of a cell: what each instance's typing found when a build last typed it,
so a later build reads an instance's demand and reports from its record instead of typing it.
the records live at `<out>/.cache/instances`. whether a record stands is decided per load
(driver/cache): its key covers the configuration, the instance's symbol and the surfaces of the
modules it reads, and its places stand while its declaring module's file places what that
module's surface keeps where it did. a store that is missing, truncated or damaged reads as
empty. the layout is `"MIR1"`, the record count, the records, then the sha-256 of everything
before it. a record is its symbol, its declaring module's name, its 32-byte key, the 32-byte
placed digest of its declaring module's file, the count and names of the modules it reads, the
count of the instances its body asks for and per instance its symbol and the offset and length
of where in the declaring module's source the body asks for it, then the length and bytes of
its reports (diagnostic.encode), every number a little-endian u32 and every name its length then
its bytes

## val NONE

```mach
pub val NONE: u32 = 0xFFFFFFFF
```

no record

## rec Child

```mach
pub rec Child;
```

an instance an instance's body asks for: its symbol and where in the body's declaring module's
source it is asked for

## rec Record

```mach
pub rec Record;
```

one instance's typing as a build recorded it

name: its symbol
origin: the name of the module declaring its generic
key: what its typing read, as driver/cache keys it
placed: the placed digest of the declaring module's file, which says where its places lie
reads: the first of the modules it reads in the store's reads, the declaring module first
children: the first of the instances its body asks for in the store's children, in the order
            asked
diags: its reports, a report placed where it is asked for placed nowhere
current: this load keys it as it was recorded, so its demand and reports stand
positional: its places stand as well
origin_id: the declaring module in this load, when it is loaded
file: the declaring module's file, where its places lie

## rec Store

```mach
pub rec Store;
```

records by symbol, the modules each reads by name and as this load numbers them, and what each
body asks for

## fun init

```mach
pub fun init(a: *A.Allocator) Store;
```

## fun dnit

```mach
pub fun dnit(st: *Store);
```

## fun find

```mach
pub fun find(st: *Store, name: intern.StrId) u32;
```

the record of the instance `name`, NONE when the store holds none

## fun at

```mach
pub fun at(st: *Store, record: u32) *Record;
```

## fun read_name

```mach
pub fun read_name(st: *Store, r: *Record, i: u32) intern.StrId;
```

the `i`th module the record reads, by name

## fun read_id

```mach
pub fun read_id(st: *Store, r: *Record, i: u32) module.ModuleId;
```

the `i`th module the record reads, as this load numbers it

## fun set_read_id

```mach
pub fun set_read_id(st: *Store, r: *Record, i: u32, mid: module.ModuleId);
```

## fun child_at

```mach
pub fun child_at(st: *Store, r: *Record, i: u32) *Child;
```

## fun add

```mach
pub fun add(st: *Store, name: intern.StrId, origin: intern.StrId, key: *[32]u8, placed: *[32]u8,
reads: *intern.StrId, reads_count: u32, children: *Child, children_count: u32, diags: *u8, diags_len: u32) res[u32, fail.Fail];
```

a record of the instance `name` added, its reads, children and report bytes copied in. a store
holds one record per symbol

## fun copy

```mach
pub fun copy(into: *Store, from: *Store, record: u32) res[u32, fail.Fail];
```

the record `record` of `from` added to `into`

## fun decode

```mach
pub fun decode(a: *A.Allocator, itn: *intern.Interner, bytes: *u8, len: usize) res[Store, fail.Fail];
```

the store `bytes` hold, empty when they are not a complete store. err only when allocation fails

## fun load

```mach
pub fun load(a: *A.Allocator, itn: *intern.Interner, file: str) res[Store, fail.Fail];
```

the store at `file`, empty when there is none or it cannot be read

## fun store

```mach
pub fun store(st: *Store, itn: *intern.Interner, out_dir: str, file: str) err[fail.Fail];
```

write the store to `file` through a sibling temporary; the encoding passes through the store's
allocator

