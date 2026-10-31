# mach.lang.driver.cache

the persistent object cache. every module the build emits is keyed once per
load, after the load walk and before anything is resolved: its key covers
the build configuration and compiler identity, the module's own source and
embedded files, and the surface of every module it imports, transitively.
an import's surface is its source without the bodies no importer compiles
(see driver/surface), so an edit inside such a body keys only its own
module. a release build inlines bodies across modules, so there an import's
whole source is its surface. a module whose `obj/` object carries its key
is reused, and when nothing the build still compiles imports it, it is not
resolved, typed or lowered at all. source digests come from the per-file
digest memo in the cell's `.cache` directory (driver/cache/memo)

## fun configuration

```mach
pub fun configuration(scratch: *A.Allocator, p: *project.Project, config: *u8, config_len: usize,
compiler_id: *compiler.Identity, digest: *[32]u8) err[fail.Fail];
```

what every module's key shares: the compiler identity, the configuration
bytes the driver captured (target, platform, project identity), the request,
the step outputs, the realized dependency closure and the link providers

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
pub fun forget(p: *project.Project);
```

every key and skip decision of the previous load is dropped: the next load
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

## fun prepare

```mach
pub fun prepare(p: *project.Project, config: *u8, config_len: usize, ph: readout.Phase) err[fail.Fail];
```

once per load, after the load walk and the embedded inputs, before anything
is resolved: key every module, read each emitted module's `obj/` object under
its key, and mark the modules the front end skips. config is the captured
configuration identity. its items are reported under the readout phase ph

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

## fun publish

```mach
pub fun publish(p: *project.Project, m: *project.ModuleEntry, o: *Object, object: query.QueryKey) err[fail.Fail];
```

a compiled object carries its record into `obj/`: the key it was built under, the facts its
lowered unit holds and the warnings building it reported. the record waits on the module
until its object is written

## fun publish_test

```mach
pub fun publish_test(p: *project.Project, m: *project.ModuleEntry, o: *Object, object: query.QueryKey) err[fail.Fail];
```

the same for the module's test object, which records only its own back half's warnings:
the normal object carries the front end's

