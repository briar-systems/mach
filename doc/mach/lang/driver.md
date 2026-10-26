# mach.lang.driver

## fwd frontend.begin_query_phase

```mach
fwd frontend.begin_query_phase
```

forwards [`mach.lang.driver.frontend.begin_query_phase`](driver/frontend.md#fun-begin_query_phase)

## fwd frontend.finish_query_phase

```mach
fwd frontend.finish_query_phase
```

forwards [`mach.lang.driver.frontend.finish_query_phase`](driver/frontend.md#fun-finish_query_phase)

## fwd frontend.refresh_diagnostics

```mach
fwd frontend.refresh_diagnostics
```

forwards [`mach.lang.driver.frontend.refresh_diagnostics`](driver/frontend.md#fun-refresh_diagnostics)

## fwd frontend.frontend_status

```mach
fwd frontend.frontend_status
```

forwards [`mach.lang.driver.frontend.frontend_status`](driver/frontend.md#fun-frontend_status)

## fwd frontend.syntax

```mach
fwd frontend.syntax
```

forwards [`mach.lang.driver.frontend.syntax`](driver/frontend.md#fun-syntax)

## fwd frontend.resolved

```mach
fwd frontend.resolved
```

forwards [`mach.lang.driver.frontend.resolved`](driver/frontend.md#fun-resolved)

## fwd frontend.typed

```mach
fwd frontend.typed
```

forwards [`mach.lang.driver.frontend.typed`](driver/frontend.md#fun-typed)

## fwd passes.codegen_reusable

```mach
fwd passes.codegen_reusable
```

forwards [`mach.lang.driver.passes.codegen_reusable`](driver/passes.md#fun-codegen_reusable)

## fwd passes.module_in_current_project

```mach
fwd passes.module_in_current_project
```

forwards [`mach.lang.driver.passes.module_in_current_project`](driver/passes.md#fun-module_in_current_project)

## fwd driver_load.entry_module_fqn

```mach
fwd driver_load.entry_module_fqn
```

forwards [`mach.lang.driver.load.entry_module_fqn`](driver/load.md#fun-entry_module_fqn)

## fwd config.resolve_run_artifact

```mach
fwd config.resolve_run_artifact
```

forwards [`mach.lang.driver.config.resolve_run_artifact`](driver/config.md#fun-resolve_run_artifact)

## fwd config.RunArtifact

```mach
fwd config.RunArtifact
```

forwards [`mach.lang.driver.config.RunArtifact`](driver/config.md#rec-runartifact)

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

## fun run_link_pass

```mach
pub fun run_link_pass(p: *project.Project) res[bool, fail.Fail];
```

## fun build_project

```mach
pub fun build_project(s: *session.Session, project_root: str, selectors: *manifest.Selectors) res[project.Project, fail.Fail];
```

load the manifest at `project_root`, begin the one cell `selectors` names in it and
load its artifact's closure

## fun closure_locate

```mach
pub fun closure_locate(s: *session.Session, m: *manifest.Manifest, project_root: str,
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
pub fun closure_checked(s: *session.Session, m: *manifest.Manifest, project_root: str,
check: *driver_closure.Check) res[project.Closure, fail.Fail];
```

locate a root manifest's dependency closure as `closure_locate` does, holding every edge
to `check` as it is located, as `mach dep verify` does

check: what each located edge is held to
ret: the closure; err naming the first edge that does not locate or does not hold

## fun begin_build

```mach
pub fun begin_build(s: *session.Session, m: *manifest.Manifest, closure: *project.Closure, req: *request.BuildRequest,
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

## fun analyze_project_tolerant

```mach
pub fun analyze_project_tolerant(s: *session.Session, m: *manifest.Manifest, req: *request.BuildRequest,
roots: project.RootSet, extra_roots: *intern.StrId, extra_root_count: u32, phase: FrontendPhase) res[project.Project, fail.Fail];
```

frontend analysis for tools: the project comes back whenever the frontend
ran, whatever its phases decided, so a consumer can read the trees, resolve
results and sema results beside the diagnostics. every module that reached
a phase carries that phase's product on its ModuleEntry even when the
module's own query was rejected; the standing is frontend.frontend_status(p).
a rejected phase ends the analysis at that phase; err is an operational
failure only (an unreadable manifest, allocation, I/O)

s: the session
m: the loaded manifest
req: the build request selecting target, profile and artifact
roots: the root set the frontend loads: `project.ROOT_ARTIFACT` is the
                  selected artifact's closure, `project.ROOT_UNION` every artifact's
                  with the modules only a foreign target reaches gated out and
                  `{artifact.<id>.out}` resolving over every artifact some artifact
                  needs
extra_roots: module fqns loaded beside the root set's entries, nil with count 0
extra_root_count: how many extra roots
phase: the last frontend phase to run
ret: the project, released by the caller with project.dnit_project

## fun run_load_phase

```mach
pub fun run_load_phase(p: *project.Project) err[fail.Fail];
```

## fun refresh_frontend

```mach
pub fun refresh_frontend(p: *project.Project, mid: session.ModuleId, phase: FrontendPhase) res[bool, fail.Fail];
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
pub fun load_manifest(s: *session.Session, project_root: str) res[manifest.Manifest, fail.Fail];
```

the root manifest of the project at `project_root`, read once: its text, an
open buffer's when the session holds one, becomes the session's input for the
file, so a warm session sees an edit to it as an input change, and the manifest
records the input and the revision it was read at

s: the session; its build allocator owns the manifest
project_root: the project root
ret: the manifest, released with `manifest.dnit`; err when it cannot be
              read, does not parse or fails a check

