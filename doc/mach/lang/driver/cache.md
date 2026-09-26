# mach.lang.driver.cache

## fun configuration

```mach
pub fun configuration(scratch: *A.Allocator, p: *project.Project, config: *u8, config_len: usize,
compiler_id: *compiler.Identity, digest: *[32]u8) err[fail.Fail];
```

what every module's key shares: the compiler identity, the configuration
bytes the driver captured (target, platform, project identity), the request,
the step outputs, the realized dependency closure and the link providers

## fun identify

```mach
pub fun identify(p: *project.Project, ph: u8) err[fail.Fail];
```

the compiler identity is a process constant: read once per project, reported
under the readout phase ph that asked for it

## fun identity_available

```mach
pub fun identity_available(p: *project.Project) bool;
```

## fun object_path

```mach
pub fun object_path(p: *project.Project, m: *project.ModuleEntry, a: *A.Allocator) res[str, fail.Fail];
```

where the build writes module m's object: `obj/<project>/<module path>.<ext>`

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
ir: the ir and asm emitters read it, and a whole-module backend reads every
referenced module's ir while generating one object

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

## rec Cached

```mach
pub rec Cached;
```

the image an `obj/` object's record carries, which is the image codegen
produced, with the facts beside it

## fun read

```mach
pub fun read(p: *project.Project, location: str, expected: *[32]u8) res[opt[Cached], fail.Fail];
```

the image the object at `location` records when its record says it was
built under `expected`, none on a miss: an object that is missing,
unreadable, carries no record or a damaged one, or was built under another
key is rebuilt, never reused. the object is parsed only to find its record,
since what a format spells is not always what the link reads. err only when
allocation fails

## fun object_key

```mach
pub fun object_key(p: *project.Project, m: *project.ModuleEntry, out: *[32]u8) err[fail.Fail];
```

the object key of a module: its module key and its name. a module's other
objects (a test object) key off this one

## fun test_object_key

```mach
pub fun test_object_key(p: *project.Project, m: *project.ModuleEntry, out: *[32]u8) err[fail.Fail];
```

the key of module m's test object: its normal object's key and its whole source

## val TEST_KEY_BIT

```mach
pub val TEST_KEY_BIT: u64 = 0x100000000
```

a module's test object is keyed in the query database by its stable id and this bit

## fun test_input_key

```mach
pub fun test_input_key(m: *project.ModuleEntry) u64;
```

the query key of a module's test object products and its object key input

## fun prepare

```mach
pub fun prepare(p: *project.Project, config: *u8, config_len: usize, ph: u8) err[fail.Fail];
```

once per load, after the load walk and the embedded inputs, before anything
is resolved: key every module, read each emitted module's `obj/` object under
its key, and mark the modules the front end skips. config is the captured
configuration identity. its items are reported under the readout phase ph

## fun take_staged

```mach
pub fun take_staged(p: *project.Project, m: *project.ModuleEntry) *of.ObjectImage;
```

hand the staged image to the query that publishes it; the facts stay with the module

## fun take_staged_test

```mach
pub fun take_staged_test(p: *project.Project, m: *project.ModuleEntry) *of.ObjectImage;
```

## fun collect_tests

```mach
pub fun collect_tests(p: *project.Project, m: *project.ModuleEntry, mid: u32, c: *testrunner.Collected) err[fail.Fail];
```

a module's tests, from its lowered test ir or from the facts its restored test object carries

## fun publish

```mach
pub fun publish(p: *project.Project, m: *project.ModuleEntry, image: *of.ObjectImage, back_key: u64) err[fail.Fail];
```

a generated image carries its record into `obj/`: the key it was built
under, the facts its lowered ir holds, and the image itself

## fun publish_test

```mach
pub fun publish_test(p: *project.Project, m: *project.ModuleEntry, image: *of.ObjectImage, back_key: u64) err[fail.Fail];
```

the same for the module's test object, which records only its own back half's
warnings: the normal object carries the front end's

