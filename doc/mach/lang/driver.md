# mach.lang.driver

## fun append_frontend_roots

```mach
pub fun append_frontend_roots(p: *project.Project, roots: *Vector[query.QueryKey]) err[fail.Fail];
```

## fun append_test_roots

```mach
pub fun append_test_roots(p: *project.Project, roots: *Vector[query.QueryKey]) err[fail.Fail];
```

the backend's roots and each test object's codegen

## fun phase_finish

```mach
pub fun phase_finish(p: *project.Project,
result: err[fail.Fail]) err[fail.Fail];
```

a phase the query engine does not run is published as one it does: the
project's diagnostics are refreshed beside its result

## fun reconfigure

```mach
pub fun reconfigure(p: *project.Project, req: request.BuildRequest) err[fail.Fail];
```

a loaded project under another request from its host: the build context is frozen again, so
what read the old one is computed again, and nothing reads a context no freeze captured

## fun run_sema_pass

```mach
pub fun run_sema_pass(p: *project.Project) err[fail.Fail];
```

## fun run_lower_pass

```mach
pub fun run_lower_pass(p: *project.Project) err[fail.Fail];
```

## fun run_codegen_pass

```mach
pub fun run_codegen_pass(p: *project.Project) err[fail.Fail];
```

the codegen stage: each emitted module's object, made by its own step or restored from `obj/`

## fun run_test_pass

```mach
pub fun run_test_pass(p: *project.Project) err[fail.Fail];
```

the test stage: each test object, lowered against its module's normal object

## fun run_link_pass

```mach
pub fun run_link_pass(p: *project.Project, job: *project.LinkJob) err[fail.Fail];
```

the link stage: the artifact `job` names, written by its action only when an input of the
link moved since it last ran or the artifact on disk is not the one it wrote

## fun closure_locate

```mach
pub fun closure_locate(s: *session.Session, m: *manifest_model.Manifest, project_root: str,
unrealized: driver_closure.Unrealized) res[project.Closure, fail.Fail];
```

locate a root manifest's dependency closure with no build cell configured, as a
build locates it: every dependency under dep/, its manifest and its direct
edges, and nothing is checked against git or the pins

s: the session; its build allocator owns the closure
m: the root manifest
project_root: the root project's directory
unrealized: what a dependency nothing is realized for is
ret: the closure, released with `project.closure_dnit`

## fun closure_checked

```mach
pub fun closure_checked(s: *session.Session, m: *manifest_model.Manifest, project_root: str,
check: *driver_closure.Check) res[project.Closure, fail.Fail];
```

locate a root manifest's dependency closure as `closure_locate` does, holding every edge
to `check` as it is located, as `mach dep verify` does

check: what each located edge is held to
ret: the closure; err naming the first edge that does not locate or does not hold

## fun begin_build

```mach
pub fun begin_build(s: *session.Session, m: *manifest_model.Manifest, closure: *project.Closure, req: *request.BuildRequest,
progress: *readout.Progress) res[project.Project, fail.Fail];
```

begin the build the request names: every goal, a test goal included, loads the
selected artifact's closure

m: the root manifest the cell was planned from
closure: `m`'s closure as the plan located it, which the project borrows

## def FrontendPhase

```mach
pub def FrontendPhase: u8
```

## val FRONTEND_PARSE

```mach
pub val FRONTEND_PARSE:   FrontendPhase = 0
```

## val FRONTEND_RESOLVE

```mach
pub val FRONTEND_RESOLVE: FrontendPhase = 1
```

## val FRONTEND_SEMA

```mach
pub val FRONTEND_SEMA:    FrontendPhase = 2
```

## rec Analysis

```mach
pub rec Analysis;
```

one frontend analysis for a tool: the project at a root, its request composed from the
manifest and the selection as `mach check` composes it, with the modules of `files`
loaded beside the root set

root: the project's directory, the one holding its mach.toml
selection: the artifact, target and profile selection, one cell of which is analyzed
roots: the root set the frontend loads: `project.ROOT_ARTIFACT` is the selected
            artifact's closure, `project.ROOT_UNION` every artifact's with the modules
            only a foreign target reaches gated out and `{artifact.<id>.out}` resolving
            over every artifact some artifact needs
files: source files whose modules load beside the root set, nil with count 0; a
            file of another project, or outside the root's source directory, is skipped
file_count: how many files
phase: the last frontend phase to run

## fun analyze

```mach
pub fun analyze(s: *session.Session, an: *Analysis) res[project.Project, fail.Fail];
```

the frontend analysis tools share: the project comes back whenever the frontend ran,
whatever its phases decided, so a consumer can read the trees, resolve results and sema
results beside the diagnostics. every module that reached a phase carries that phase's
product on its ModuleEntry even when the module's own query was rejected; the standing is
frontend.frontend_status(p). a rejected phase ends the analysis at that phase; err is an
operational failure only (an unreadable manifest, a selection naming no cell, allocation,
I/O). every string `an` holds is borrowed for the call, and the project borrows `root`

s: the session
an: what to analyze
ret: the project, released by the caller with project.dnit_project

## fun run_load_phase

```mach
pub fun run_load_phase(p: *project.Project) err[fail.Fail];
```

## fun refresh_frontend

```mach
pub fun refresh_frontend(p: *project.Project, mid: module.ModuleId, phase: FrontendPhase) res[bool, fail.Fail];
```

re-derive a loaded project's frontend after one module's text changed, without
reloading its closure. the module is reparsed and re-walked in place; when its load
surface survived (driver_load.module_reload), the module set and topo are as loaded and
the query phases rerun over them, so every unchanged module's resolve and sema are
hits and only the edited module and its dependents recompute. a surface that did
not survive is reported as ok(false): the caller reloads

p: the loaded project, at least at FRONTEND_RESOLVE
mid: the module whose text changed
phase: the frontend phase to re-derive to, at most the one the project was loaded to
ret: ok(true) when refreshed, a rejection included since the project's standing carries it;
       ok(false) when the caller must reload; or the internal failure

## fun load_manifest

```mach
pub fun load_manifest(s: *session.Session, project_root: str) res[manifest_model.Manifest, fail.Fail];
```

the root manifest of the project at `project_root`, read once: its text, an
open buffer's when the session holds one, becomes the session's input for the
file, so a warm session sees an edit to it as an input change, and the manifest
records the input and the revision it was read at

s: the session; its build allocator owns the manifest
project_root: the project root
ret: the manifest, released with `manifest_model.dnit`; err when it cannot be
              read, does not parse or fails a check

## fun project_root_of

```mach
pub fun project_root_of(alloc: *A.Allocator, file_path: str) res[opt[str], fail.Fail];
```

the root of the project a file belongs to: the nearest directory above it holding a
mach.toml, none when no directory does

## fun module_under

```mach
pub fun module_under(alloc: *A.Allocator, itn: *intern.Interner, path: str, root: str, id_text: str,
src_text: str) res[opt[intern.StrId], fail.Fail];
```

the module the file at `path` is under the project at `root`: none when it lies outside the
source directory

id_text: the project id, the module path's first segment
src_text: the manifest's source directory, relative to `root`

## fun file_module

```mach
pub fun file_module(alloc: *A.Allocator, itn: *intern.Interner, path: str, root: str, id_text: str,
src_text: str) res[opt[intern.StrId], fail.Fail];
```

the module the file at `path` is loaded as beside the project at `root`: none when the
file belongs to another project or lies outside the source directory

id_text: the project id, the module path's first segment
src_text: the manifest's source directory, relative to `root`

