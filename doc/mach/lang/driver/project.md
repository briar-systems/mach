# mach.lang.driver.project

## def RootSet

```mach
pub def RootSet: u8
```

## val ROOT_ARTIFACT

```mach
pub val ROOT_ARTIFACT: RootSet = 0
```

## val ROOT_TEST

```mach
pub val ROOT_TEST: RootSet = 1
```

## val ROOT_UNION

```mach
pub val ROOT_UNION: RootSet = 2
```

## def BuildMode

```mach
pub def BuildMode: u8
```

## val MODE_EXECUTABLE

```mach
pub val MODE_EXECUTABLE: BuildMode = 0
```

## val MODE_LIBRARY

```mach
pub val MODE_LIBRARY: BuildMode = 1
```

## def TargetOpt

```mach
pub def TargetOpt: u8
```

## val TARGET_OPT_DEFAULT

```mach
pub val TARGET_OPT_DEFAULT: TargetOpt = 0
```

## val TARGET_OPT_DEBUG

```mach
pub val TARGET_OPT_DEBUG: TargetOpt = 1
```

## val TARGET_OPT_RELEASE

```mach
pub val TARGET_OPT_RELEASE: TargetOpt = 2
```

## rec TargetEntry

```mach
pub rec TargetEntry;
```

## rec DepEntry

```mach
pub rec DepEntry;
```

## rec ProjectConfig

```mach
pub rec ProjectConfig;
```

what a configured build cell compiles and where it writes

project_root: the directory of the project that declares the cell: the root
                  project, or for a dependency's cell the dependency's realization
home_root: the root project's directory; `project_out` is rooted here
owner: the dependency that declares the cell, moved out of the closure;
                  meaningful only with `dependency_owned`
dependency_owned: the cell is a requirement of a dependency's default library
                  artifacts, built for the root project
deps: the realized closure the cell compiles against: the root's whole
                  closure, or a dependency cell's own transitive closure
art_reqs: the requirements of the cell's artifact, in its declaring manifest
req_scopes: one requirement scope per project the cell compiles, the declaring
                  project's first

## rec PubConst

```mach
pub rec PubConst;
```

## rec ModRef

```mach
pub rec ModRef;
```

## rec TupleGateResult

```mach
pub rec TupleGateResult;
```

## rec ModuleEntry

```mach
pub rec ModuleEntry;
```

## def LoadStatus

```mach
pub def LoadStatus: u8
```

## val LOAD_NEW

```mach
pub val LOAD_NEW: LoadStatus = 0
```

## val LOAD_LOADING

```mach
pub val LOAD_LOADING: LoadStatus = 1
```

## val LOAD_DONE

```mach
pub val LOAD_DONE: LoadStatus = 2
```

## rec TargetTuple

```mach
pub rec TargetTuple;
```

## rec CacheKey

```mach
pub rec CacheKey;
```

an entry this unit build restored or published, which eviction keeps

## rec RawLowerCapture

```mach
pub rec RawLowerCapture;
```

## rec Project

```mach
pub rec Project;
```

## val QUERY_KIND_SLOTS

```mach
pub val QUERY_KIND_SLOTS: u32 = 32
```

## fun note_compute

```mach
pub fun note_compute(p: *Project, kind: query.QueryKind);
```

## fun begin_query_phase

```mach
pub fun begin_query_phase(p: *Project) res[query.Operation, outcome.Fail];
```

## fun refresh_diagnostics

```mach
pub fun refresh_diagnostics(p: *Project) err[outcome.Fail];
```

## fun finish_query_phase

```mach
pub fun finish_query_phase(p: *Project, operation: query.Operation,
roots: *query.QueryKey, count: usize,
result: err[outcome.Fail]) err[outcome.Fail];
```

## fun span_eq_str

```mach
pub fun span_eq_str(source: str, span: token.Span, s: str) bool;
```

## fun link_name_for

```mach
pub fun link_name_for(p: *Project, bare: intern.StrId) res[intern.StrId, fail.Fail];
```

## fun release_staged

```mach
pub fun release_staged(p: *Project, m: *ModuleEntry);
```

staged images are project-owned until a query takes them, object_image is borrowed

## fun release_all_staged

```mach
pub fun release_all_staged(p: *Project);
```

## fun dnit_project

```mach
pub fun dnit_project(p: *Project);
```

## fun init_project

```mach
pub fun init_project(p: *Project, s: *session.Session);
```

## fun free_dep_entries

```mach
pub fun free_dep_entries(alloc: *A.Allocator, deps: *DepEntry, count: u32, capacity: usize);
```

## fun free_dep_entries_in_place

```mach
pub fun free_dep_entries_in_place(alloc: *A.Allocator, deps: *DepEntry, count: u32);
```

release what each entry owns, leaving the entries' own storage to the caller

## fun free_artifact_reqs

```mach
pub fun free_artifact_reqs(alloc: *A.Allocator, reqs: *manifest.ArtifactReq, count: u32);
```

## fun map_opt

```mach
pub fun map_opt(o: manifest.MOpt) opt[TargetOpt];
```

the pipeline level a manifest profile level selects; absent for a tag
outside the catalog, which the caller reports through the catalog policy

## fun intern_unwrap

```mach
pub fun intern_unwrap(itn: *intern.Interner, text: str) intern.StrId;
```

## fun intern_opt_unwrap

```mach
pub fun intern_opt_unwrap(itn: *intern.Interner, text: str) intern.StrId;
```

## fun fqn_name

```mach
pub fun fqn_name(p: *Project, fqn: intern.StrId) str;
```

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
many modules load after it

## fun module_reserve

```mach
pub fun module_reserve(p: *Project) err[fail.Fail];
```

make room for one more entry so the next module_append cannot fail

## fun module_append

```mach
pub fun module_append(p: *Project, m: ModuleEntry) session.ModuleId;
```

publish an entry under the next ModuleId into the slot module_reserve made

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
pub fun module_by_file(p: *Project, fid: src.FileId) opt[session.ModuleId];
```

the loaded module a source file backs, by a scan of the module table: editor edits
arrive as file ids and the table is small beside the work a refresh saves

p: the project
fid: the file
ret: the module's id, or none when no loaded module reads that file

## fun drop_surface

```mach
pub fun drop_surface(a: *A.Allocator, slot: **sema.ModuleSema);
```

