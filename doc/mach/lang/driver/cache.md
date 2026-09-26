# mach.lang.driver.cache

## fun snapshot

```mach
pub fun snapshot(scratch: *A.Allocator, p: *project.Project, config: *u8, config_len: usize,
compiler_id: *compiler.Identity, digest: *[32]u8) err[fail.Fail];
```

hash the whole active build cell because generic instantiations can cross import direction

## fun operation_active

```mach
pub fun operation_active(p: *project.Project) bool;
```

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

## fun prepare

```mach
pub fun prepare(p: *project.Project, snapshot: *[32]u8) err[fail.Fail];
```

once per query operation. snapshot is the cell digest the operation computed,
nil when the cache is off or the compiler has no identity

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

the module's codegen product is its cached object under the current
snapshot, whether it is still staged or a query already took it: the module
does not lower, and its object is already in `obj/`

## fun staged_restored

```mach
pub fun staged_restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

restored and still staged, which is what the codegen query publishes

## fun test_restored

```mach
pub fun test_restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

the module's test object is its cached object under the current snapshot:
it does not lower, and it is already in `obj/`

## fun test_staged_restored

```mach
pub fun test_staged_restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

the test object is restored and still staged for the codegen query to publish

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

## fun restore

```mach
pub fun restore(p: *project.Project, mid: session.ModuleId, ph: u8) res[bool, fail.Fail];
```

a hit is reported under the readout phase ph that asked for the module

## fun restore_test

```mach
pub fun restore_test(p: *project.Project, mid: session.ModuleId, ph: u8) res[bool, fail.Fail];
```

the module's test object, read back like its normal object

## fun take_staged

```mach
pub fun take_staged(p: *project.Project, m: *project.ModuleEntry) *of.ObjectImage;
```

hand the staged image to the query that publishes it; the facts stay with the module

## fun take_staged_test

```mach
pub fun take_staged_test(p: *project.Project, m: *project.ModuleEntry) *of.ObjectImage;
```

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

a module's tests, from its lowered test ir or from the facts its restored test object carries

## fun publish

```mach
pub fun publish(p: *project.Project, m: *project.ModuleEntry, image: *of.ObjectImage) err[fail.Fail];
```

a generated image carries its record into `obj/`: the key it was built
under and the facts its lowered ir holds

## fun publish_test

```mach
pub fun publish_test(p: *project.Project, m: *project.ModuleEntry, image: *of.ObjectImage) err[fail.Fail];
```

