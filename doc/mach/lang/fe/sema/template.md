# mach.lang.fe.sema.template

the template part of a module's interface: what an importer's lowering reads to emit an
instance of one of the module's generic, comptime-parameter or pack functions. it holds an
owned copy of the module's syntax, its resolution, its typing and the constants each stage
bound, every node at the id it has in the module, so an importer's instance typing, keyed by
those ids, reads against it. a module with no such function holds none

## rec Typing

```mach
pub rec Typing;
```

the module's own typing of its syntax, by node id

## rec Template

```mach
pub rec Template;
```

load, resolved, typed: the constants and decisions the load, resolve and sema stages bound
env: the build and module the module's comptime reads

## fun none

```mach
pub fun none(alloc: *A.Allocator) Template;
```

## fun dnit

```mach
pub fun dnit(t: *Template);
```

## rec Captured

```mach
pub rec Captured;
```

what a template is captured from, all of which it copies

## fun capture

```mach
pub fun capture(alloc: *A.Allocator, from: Captured) res[Template, fail.Fail];
```

an owned copy of everything `from` names, from `alloc`

