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

an object read back from `obj/` with the facts its record carries

## fun read

```mach
pub fun read(p: *project.Project, location: str, expected: *[32]u8) res[opt[Cached], fail.Fail];
```

the object at `location` when its record says it was built under `expected`,
none on a miss: an object that is missing, unreadable, carries no record or
a damaged one, or was built under another key is rebuilt, never reused.
err only when allocation fails

## fun object_key

```mach
pub fun object_key(p: *project.Project, m: *project.ModuleEntry, out: *[32]u8) err[fail.Fail];
```

the object key of a module: its module key and its name. a module's other
objects (a test object) key off this one

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

## fun scalarized

```mach
pub fun scalarized(p: *project.Project, m: *project.ModuleEntry) u32;
```

what the engine reads from lowered ir, answered from the ir when the module
lowered and from the restored facts when it did not

## fun collect_tests

```mach
pub fun collect_tests(p: *project.Project, m: *project.ModuleEntry, mid: u32, c: *testrunner.Collected) err[fail.Fail];
```

## fun publish

```mach
pub fun publish(p: *project.Project, m: *project.ModuleEntry, image: *of.ObjectImage) err[fail.Fail];
```

a generated image carries its record into `obj/`: the key it was built
under and the facts its lowered ir holds

