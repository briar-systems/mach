# mach.lang.driver.project

## def RootSet

```mach
pub def RootSet: u8
```

## val ROOT_ARTIFACT

```mach
pub val ROOT_ARTIFACT: RootSet = 0
```

## val ROOT_UNION

```mach
pub val ROOT_UNION: RootSet = 1
```

## rec TargetEntry

```mach
pub rec TargetEntry;
```

## fun target_entry_of

```mach
pub fun target_entry_of(bu: *manifest.BuildUnit) TargetEntry;
```

the target entry a selected build unit describes: the one place a TargetEntry is
built, so every field is carried wherever an entry is made

## rec DepEntry

```mach
pub rec DepEntry;
```

## rec Closure

```mach
pub rec Closure;
```

a root manifest's located dependency closure: every dependency realized under
dep/, its manifest read once, and its direct edges. a plan locates it once and
every cell it configures borrows it; a project configured without one locates
and owns its own

alloc: owns the entries, their manifests and their edges

## fun closure_empty

```mach
pub fun closure_empty(alloc: *A.Allocator) Closure;
```

a closure that holds no dependency

## fun closure_dnit

```mach
pub fun closure_dnit(c: *Closure);
```

## rec ProjectConfig

```mach
pub rec ProjectConfig;
```

what a configured build cell compiles and where it writes

project_root: the directory of the project that declares the cell: the root
                  project, or for a dependency's cell the dependency's realization
home_root: the root project's directory; `project_work` is rooted here
owner: the dependency that declares the cell, as the closure holds it;
                  meaningful only with `dependency_owned`
dependency_owned: the cell is a requirement of a dependency's default library
                  artifacts, built for the root project
closure: the closure this project located itself, when it was configured
                  without one to borrow; empty otherwise
deps: the realized closure the cell compiles against: the root's whole
                  closure, or a dependency cell's own transitive closure. the array
                  is the project's, each entry borrowed from the closure
art_reqs: the requirements of the cell's artifact, in its declaring manifest
req_scopes: one requirement scope per project the cell compiles, the declaring
                  project's first

## rec ArtifactEntry

```mach
pub rec ArtifactEntry;
```

an artifact entry the union walk loads: the entry module, the artifact it is of and
where the artifact's `entry` is written in the manifest

## rec ModuleEntry

```mach
pub rec ModuleEntry;
```

a module the project loaded: its identity, what the load decided for it, and the codegen
staging of its objects. its front-end products are the query engine's, keyed by its file and
its stable id, and read through mach.lang.driver.frontend

## rec Record

```mach
pub rec Record;
```

the cache record a compiled object carries into `obj/`, nil until the cache publishes one

## fun record_release

```mach
pub fun record_release(p: *Project, r: *Record);
```

give back a record's bytes

## rec TestObject

```mach
pub rec TestObject;
```

a module's test object, beside its normal object and never changing it: the
module's tests and what they reach that the normal object does not define. the
fields mirror the normal object's on ModuleEntry

## rec Project

```mach
pub rec Project;
```


since: the first query operation this project ran; a product read between its
                operations counts only when one of them reached it
context_digest: the fingerprint of the build context, once frozen
context_frozen: the build context is frozen, so a change to it is refused

## fun link_name_for

```mach
pub fun link_name_for(p: *Project, bare: intern.StrId) res[intern.StrId, fail.Fail];
```

## fun release_staged

```mach
pub fun release_staged(p: *Project, m: *ModuleEntry);
```

a staged object is the project's until a query takes it, and `object` is borrowed

## fun release_staged_test

```mach
pub fun release_staged_test(p: *Project, m: *ModuleEntry);
```

## fun release_all_staged

```mach
pub fun release_all_staged(p: *Project);
```

## fun release_fresh_staged

```mach
pub fun release_fresh_staged(p: *Project);
```

a restored product stays staged across operations: it is keyed by the key
it was read under and the next load discards it if that changed. a freshly
generated image staged by a codegen worker belongs to the operation that
generated it and leaves with it

## fun lower_scope_drop

```mach
pub fun lower_scope_drop(m: *ModuleEntry);
```

release the scope lowering bound a module's constants in

## fun dnit_project

```mach
pub fun dnit_project(p: *Project);
```

release a project and everything it owns; a nil project holds nothing

## fun set_origin

```mach
pub fun set_origin(p: *Project, o: diagnostic.Origin);
```

attribute what the project's phases report directly, outside a query, to the
phase now running: both the project's store and the session's

## fun init_project

```mach
pub fun init_project(p: *Project, s: *session.Session);
```

## fun free_dep_entries

```mach
pub fun free_dep_entries(alloc: *A.Allocator, deps: *DepEntry, count: u32, capacity: usize);
```

release `count` dependency entries and their array of `capacity`; a nil array owns
nothing

## fun module_named

```mach
pub fun module_named(p: *Project, fqn: str) session.ModuleId;
```

the module loaded under `fqn`, nil when none is

## fun module_count

```mach
pub fun module_count(p: *Project) u32;
```

the loaded module count; the next ModuleId to be issued

## fun module_at

```mach
pub fun module_at(p: *Project, mid: u32) *ModuleEntry;
```

the entry behind a ModuleId the project has issued

the store is a handle.StableChunks: an entry is written once into a fixed
chunk and never moves, so the pointer stays valid until dnit_project however
many modules load after it. total: an id the project never issued stops the
compiler

## fun module_reserve

```mach
pub fun module_reserve(p: *Project) err[fail.Fail];
```

make room for one more entry so the next module_append cannot fail

## fun module_append

```mach
pub fun module_append(p: *Project, m: ModuleEntry) session.ModuleId;
```

publish an entry under the next ModuleId into the slot module_reserve made.
total: a caller that reserved nothing first stops the compiler

## fun dep_count

```mach
pub fun dep_count(m: *ModuleEntry) u32;
```

how many modules a loaded module imports

## fun dep_at

```mach
pub fun dep_at(p: *Project, m: *ModuleEntry, i: u32) session.ModuleId;
```

the module a loaded module imports at `i`, as its load record names it

## fun module_by_fqn

```mach
pub fun module_by_fqn(p: *Project, fqn: intern.StrId) opt[*ModuleEntry];
```

the entry loaded under a fully qualified name

the returned `*ModuleEntry` is stable until dnit_project: the store never
moves an entry when more modules load, so a holder may keep it across a
nested load and read the same entry afterwards

## fun module_by_file

```mach
pub fun module_by_file(p: *Project, fid: lang_source.FileId) opt[session.ModuleId];
```

the loaded module a source file backs, by a scan of the module table: editor edits
arrive as file ids and the table is small beside the work a refresh saves

p: the project
fid: the file
ret: the module's id, or none when no loaded module reads that file

