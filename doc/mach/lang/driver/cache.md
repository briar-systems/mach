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
pub fun identify(p: *project.Project) err[fail.Fail];
```

the compiler identity is a process constant: read once per project

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

## fun early_restore_enabled

```mach
pub fun early_restore_enabled(p: *project.Project) bool;
```

a module whose object the store holds skips lowering when nothing else needs
its ir: the ir and asm emitters read it, and a whole-module backend reads
every referenced module's ir while generating one object

## fun restored

```mach
pub fun restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

the module's codegen product came from the store under the current snapshot,
whether it is still staged or a query already took it: the module does not lower

## fun staged_restored

```mach
pub fun staged_restored(p: *project.Project, m: *project.ModuleEntry) bool;
```

restored and still staged, which is what the codegen query publishes

## fun restore

```mach
pub fun restore(p: *project.Project, mid: session.ModuleId) res[bool, fail.Fail];
```

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

