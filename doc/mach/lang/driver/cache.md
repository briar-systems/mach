# mach.lang.driver.cache

the persistent object cache. every module the build emits is keyed once per
load, after the load walk and before anything is resolved: its key covers
the build configuration and compiler identity, the module's own source and
embedded files, and the surface of every module it imports, transitively.
an import's surface is its source without the bodies no importer compiles
(see driver/surface), so an edit inside such a body keys only its own
module. a release build inlines bodies across modules, so there an import's
whole source is its surface. a module whose `obj/` object carries its key
is reused: it is not lowered or compiled, and when its record stands for its
whole source and nothing the build still compiles imports it, its front end
is skipped: its record stands in for the instances its code asks for and for
its front end's warnings. an instance unit is keyed by its members and the
surfaces of the modules they read, and its object is kept in `obj/` beside
its origin's under that key. each instance's typing is recorded in the cell's
instance records (driver/cache/instances), keyed by the configuration, its
symbol and the surfaces of the modules it reads, so an instance whose record
stands is not typed. a skipped module, or an instance only records name, is
typed only when something that must be rebuilt from its typed form needs it.
source digests come from the per-file digest memo in the cell's `.cache`
directory (driver/cache/memo)

## fun configuration

```mach
pub fun configuration(scratch: *A.Allocator, p: *project.Project, config: *u8, config_len: usize,
compiler_id: *compiler.Identity, digest: *[32]u8) err[fail.Fail];
```

what every module's key shares: the compiler identity, the configuration
bytes the driver captured (target, platform, project identity), the request,
the step outputs, the realized dependency closure, the link providers and
the link requirements

## fun test_object_path

```mach
pub fun test_object_path(p: *project.Project, m: *project.ModuleEntry, a: *A.Allocator) res[str, fail.Fail];
```

where the build writes module m's test object: `obj/<project>/<module path>.test.<ext>`

## fun has_test_object

```mach
pub fun has_test_object(p: *project.Project, m: *project.ModuleEntry) bool;
```

the module gets a test object in this build: a test build, and a module whose
source declares a test or a `#[testing]` declaration

## fun early_restore_enabled

```mach
pub fun early_restore_enabled(p: *project.Project) bool;
```

a module whose object is cached skips lowering when nothing else needs its
unit: the ir and asm emitters read it

## fun restored

```mach
pub fun restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

the module's codegen product is its cached object under the key this load
built, whether it is still staged or a query already took it

## fun staged_restored

```mach
pub fun staged_restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

restored and still staged, which is what the codegen query publishes

## fun test_restored

```mach
pub fun test_restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

the module's test object is its cached object under the key this load built

## fun test_staged_restored

```mach
pub fun test_staged_restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

the test object is restored and still staged for the codegen query to publish

## fun skips_lowering

```mach
pub fun skips_lowering(p: *project.Project, m: *project.ModuleEntry) bool;
```

the module is restored and nothing needs its lowered ir

## fun lowered_count

```mach
pub fun lowered_count(p: *project.Project) u32;
```

the emitted modules the build lowers

## fun generated_count

```mach
pub fun generated_count(p: *project.Project) u32;
```

the emitted modules the build generates code for

## fun frontend_count

```mach
pub fun frontend_count(p: *project.Project) u32;
```

the modules the front end still processes

## fun forget

```mach
pub fun forget(p: *project.Project) err[fail.Fail];
```

every key, skip decision and record of the previous load is dropped: the next load
keys again, and a project that is not keyed processes every module

## fun read

```mach
pub fun read(p: *project.Project, location: str, expected: *[32]u8) res[opt[Cached], fail.Fail];
```

the object at `location` when its record says it was built under `expected`, none on a
miss: an object that is missing, unreadable, carries no record or a damaged one, or was
built under another key is rebuilt, never reused. err only when allocation fails

## val TEST_KEY_BIT

```mach
pub val TEST_KEY_BIT: u64 = 0x100000000
```

a module's test object is keyed in the query database by its stable id and this bit

## val OBJECT_KEY

```mach
pub val OBJECT_KEY: query.Kind = query.Kind;
```

the key a module's reused object was built under, keyed by its stable id and the test bit

## fun test_input_key

```mach
pub fun test_input_key(m: *project.ModuleEntry) u64;
```

the query key of a module's test object products and its object key input

## val DEMAND_RECORDS

```mach
pub val DEMAND_RECORDS: query.Kind = query.Kind;
```

the record of what the build's demand reads outside the query engine, which the demand depends
on: whether the load is keyed, each skipped module and the key its record stands under, and each
instance record and what the load decided of it. a load that is not keyed reads nothing

## fun instance_key

```mach
pub fun instance_key(p: *project.Project, name: intern.StrId, reads: *module.ModuleId, count: usize, out: *[32]u8) res[bool, fail.Fail];
```

the key an instance's record stands under: the configuration, the instance's symbol, and the
surface for importers of each module it reads, its declaring module first. a surface folds the
surfaces of what its module imports, so the key covers every typed surface the instance's
typing reads (frontend.instance_deps). false when a module it reads is not keyed

## fun prepare

```mach
pub fun prepare(p: *project.Project, config: *u8, config_len: usize, ph: readout.Phase) err[fail.Fail];
```

once per load, after the load walk and the embedded inputs, before anything
is resolved: key every module, read each emitted module's `obj/` object
under its key, mark the modules the front end skips and read the cell's
instance records. config is the captured configuration identity. its items
are reported under the readout phase ph

## fun take_staged

```mach
pub fun take_staged(p: *project.Project, m: *project.ModuleEntry) *Object;
```

hand the staged object to the query that publishes it; the facts stay with the module

## fun take_staged_test

```mach
pub fun take_staged_test(p: *project.Project, m: *project.ModuleEntry) *Object;
```

## fun collect_tests

```mach
pub fun collect_tests(p: *project.Project, m: *project.ModuleEntry, mid: u32, c: *testrunner.Collected) err[fail.Fail];
```

a module's tests, from its lowered test unit or from the facts its restored test object carries

## fun source_table

```mach
pub fun source_table(p: *project.Project) diagnostic.SourceTable;
```

the session's files named by path, as a recorded store names them

## fun publish

```mach
pub fun publish(p: *project.Project, m: *project.ModuleEntry, o: *Object, asked: *cache_record.Asked, asked_count: u32,
defined: *intern.StrId, defined_count: u32, object: query.QueryKey) err[fail.Fail];
```

a generated object carries its record into `obj/`: its key, the source it was built from, its
warnings, and what its lowering kept: the instances its code asks for and, for a module that
declares tests, the symbols it defines, which its test object declares

## fun publish_test

```mach
pub fun publish_test(p: *project.Project, m: *project.ModuleEntry, o: *Object, object: query.QueryKey) err[fail.Fail];
```

the same for the module's test object, which records only its own back half's warnings:
the normal object carries the front end's

## val UNIT_OBJECT_KEY

```mach
pub val UNIT_OBJECT_KEY: query.Kind = query.Kind;
```

the key an instance unit's object is built under, keyed by the unit's query key: the
persistent key's in-process image, as OBJECT_KEY is a module's

## rec UnitKeying

```mach
pub rec UnitKeying;
```

an instance unit's key as its members are added

## fun unit_begin

```mach
pub fun unit_begin(p: *project.Project, name: intern.StrId) UnitKeying;
```

begin the key of the unit named `name`: the configuration every key of this load shares and
the unit's name. a load that keyed nothing keys no unit

## fun unit_member

```mach
pub fun unit_member(p: *project.Project, k: *UnitKeying, name: intern.StrId, reads: *module.ModuleId, count: usize);
```

add the member named `name`, reading the modules `reads` holds, its declaring module first:
their surfaces for importers cover what its typing and lowering read of them, transitively. a
member reading a module this load did not key leaves the unit unkeyed

## fun unit_finish

```mach
pub fun unit_finish(p: *project.Project, k: *UnitKeying, u: *project.InstanceUnit) err[fail.Fail];
```

finish the unit's key into `u`, and publish it for the unit's codegen product

## fun restore_unit

```mach
pub fun restore_unit(p: *project.Project, u: *project.InstanceUnit, location: str) res[bool, fail.Fail];
```

read unit u's object at `location` under its key; a hit is staged for its codegen product and
its back half's warnings are reported again. its recorded warnings stand for the origin's
source they were reported in, as a module's do

## fun take_staged_unit

```mach
pub fun take_staged_unit(u: *project.InstanceUnit) *Object;
```

hand unit u's staged object to the product that publishes it

## fun publish_unit

```mach
pub fun publish_unit(p: *project.Project, u: *project.InstanceUnit, object: query.QueryKey) err[fail.Fail];
```

a compiled unit's object carries its record into `obj/`: its key and the warnings its own
codegen product reported, which are its lowering's and its back end's

## fun units_compiled

```mach
pub fun units_compiled(p: *project.Project) u32;
```

the instance units the build compiles rather than restores

## fun store_records

```mach
pub fun store_records(p: *project.Project, d: *demand.Demand) err[fail.Fail];
```

the cell's instance records after a demand settled: each instance of its closure keeps the
record this build made of it, or else the one that stood in for its typing, and an instance the
closure no longer holds drops out. a store that would hold what it holds is left as it is

