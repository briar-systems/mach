# mach.lang.driver

## fwd project.Project

```mach
fwd project.Project
```

forwards [`mach.lang.driver.project.Project`](driver/project.md#rec-project)

## fwd project.ModuleEntry

```mach
fwd project.ModuleEntry
```

forwards [`mach.lang.driver.project.ModuleEntry`](driver/project.md#rec-moduleentry)

## fwd project.TargetEntry

```mach
fwd project.TargetEntry
```

forwards [`mach.lang.driver.project.TargetEntry`](driver/project.md#rec-targetentry)

## fwd project.MODE_LIBRARY

```mach
fwd project.MODE_LIBRARY
```

forwards [`mach.lang.driver.project.MODE_LIBRARY`](driver/project.md#val-mode_library)

## fwd project.TARGET_OPT_RELEASE

```mach
fwd project.TARGET_OPT_RELEASE
```

forwards [`mach.lang.driver.project.TARGET_OPT_RELEASE`](driver/project.md#val-target_opt_release)

## fwd project.dnit_project

```mach
fwd project.dnit_project
```

forwards [`mach.lang.driver.project.dnit_project`](driver/project.md#fun-dnit_project)

## fwd project.fqn_name

```mach
fwd project.fqn_name
```

forwards [`mach.lang.driver.project.fqn_name`](driver/project.md#fun-fqn_name)

## fwd project.module_by_fqn

```mach
fwd project.module_by_fqn
```

forwards [`mach.lang.driver.project.module_by_fqn`](driver/project.md#fun-module_by_fqn)

## fwd project.module_count

```mach
fwd project.module_count
```

forwards [`mach.lang.driver.project.module_count`](driver/project.md#fun-module_count)

## fwd project.module_at

```mach
fwd project.module_at
```

forwards [`mach.lang.driver.project.module_at`](driver/project.md#fun-module_at)

## fwd project.link_name_for

```mach
fwd project.link_name_for
```

forwards [`mach.lang.driver.project.link_name_for`](driver/project.md#fun-link_name_for)

## fwd project.begin_query_phase

```mach
fwd project.begin_query_phase
```

forwards [`mach.lang.driver.project.begin_query_phase`](driver/project.md#fun-begin_query_phase)

## fwd project.finish_query_phase

```mach
fwd project.finish_query_phase
```

forwards [`mach.lang.driver.project.finish_query_phase`](driver/project.md#fun-finish_query_phase)

## fwd project.refresh_diagnostics

```mach
fwd project.refresh_diagnostics
```

forwards [`mach.lang.driver.project.refresh_diagnostics`](driver/project.md#fun-refresh_diagnostics)

## fwd passes.codegen_reusable

```mach
fwd passes.codegen_reusable
```

forwards [`mach.lang.driver.passes.codegen_reusable`](driver/passes.md#fun-codegen_reusable)

## fwd passes.CodeSources

```mach
fwd passes.CodeSources
```

forwards [`mach.lang.driver.passes.CodeSources`](driver/passes.md#rec-codesources)

## fwd passes.code_sources

```mach
fwd passes.code_sources
```

forwards [`mach.lang.driver.passes.code_sources`](driver/passes.md#fun-code_sources)

## fwd passes.sources_dnit

```mach
fwd passes.sources_dnit
```

forwards [`mach.lang.driver.passes.sources_dnit`](driver/passes.md#fun-sources_dnit)

## fwd passes.module_in_current_project

```mach
fwd passes.module_in_current_project
```

forwards [`mach.lang.driver.passes.module_in_current_project`](driver/passes.md#fun-module_in_current_project)

## fwd passes.debug_info_of

```mach
fwd passes.debug_info_of
```

forwards [`mach.lang.driver.passes.debug_info_of`](driver/passes.md#fun-debug_info_of)

## fwd passes.frontend_status

```mach
fwd passes.frontend_status
```

forwards [`mach.lang.driver.passes.frontend_status`](driver/passes.md#fun-frontend_status)

## fwd load.entry_module_fqn

```mach
fwd load.entry_module_fqn
```

forwards [`mach.lang.driver.load.entry_module_fqn`](driver/load.md#fun-entry_module_fqn)

## fwd dcfg.resolve_run_artifact

```mach
fwd dcfg.resolve_run_artifact
```

forwards [`mach.lang.driver.config.resolve_run_artifact`](driver/config.md#fun-resolve_run_artifact)

## fwd dcfg.RunArtifact

```mach
fwd dcfg.RunArtifact
```

forwards [`mach.lang.driver.config.RunArtifact`](driver/config.md#rec-runartifact)

## fwd mach.lang.driver.registry.setup_registry

```mach
fwd mach.lang.driver.registry.setup_registry
```

forwards [`mach.lang.driver.registry.setup_registry`](driver/registry.md#fun-setup_registry)

## fun append_frontend_roots

```mach
pub fun append_frontend_roots(p: *project.Project, roots: *Vector[query.QueryKey]) err[outcome.Fail];
```

## fun append_test_roots

```mach
pub fun append_test_roots(p: *project.Project, roots: *Vector[query.QueryKey]) err[outcome.Fail];
```

the backend's roots and each test object's lowering and codegen

## fun run_sema_pass

```mach
pub fun run_sema_pass(p: *project.Project) err[outcome.Fail];
```

## fun run_lower_pass

```mach
pub fun run_lower_pass(p: *project.Project) err[outcome.Fail];
```

## fun run_link_pass

```mach
pub fun run_link_pass(p: *project.Project) res[bool, outcome.Fail];
```

## fun build_project

```mach
pub fun build_project(s: *session.Session, project_root: str, pick: *manifest.Selection) res[project.Project, outcome.Fail];
```

## fun verify_dependencies

```mach
pub fun verify_dependencies(s: *session.Session, project_root: str, release: bool,
overrides: *Vector[ddep.RootOverride]) err[outcome.Fail];
```

check a project's realized dependency closure without changing it

s: the session
project_root: the root project's directory
release: also hold the root to the release rule: every dependency selected by
              `version` or an exact `tag/`, as a release about to be tagged must be
overrides: when not nil, receives every requirer selector a root override replaced
ret: err naming the first mismatch

## fun realize_closure

```mach
pub fun realize_closure(s: *session.Session, m: *manifest.Manifest, project_root: str) res[project.Project, outcome.Fail];
```

resolve and verify a root manifest's dependency closure with no build cell
configured: `p.config.deps` holds every realized dependency, its manifest and
its direct edges, exactly as a build configures them

s: the session
m: the root manifest
project_root: the root project's directory
ret: the project holding the closure, released with `project.dnit_project`;
              err from dependency resolution, its diagnostics published

## fun begin_build

```mach
pub fun begin_build(s: *session.Session, m: *manifest.Manifest, req: *request.BuildRequest, ev: *readout.Progress) res[project.Project, outcome.Fail];
```

begin the build the request names: every goal, a test goal included, loads the
selected artifact's closure

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
roots: project.RootSet, extra_roots: *intern.StrId, extra_root_count: u32, phase: FrontendPhase) res[project.Project, outcome.Fail];
```

frontend analysis for tools: the project comes back whenever the frontend
ran, whatever its phases decided, so a consumer can read the trees, resolve
results and sema results beside the diagnostics. every module that reached
a phase carries that phase's product on its ModuleEntry even when the
module's own query was rejected; the standing is passes.frontend_status(p).
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

## fun run_steps_phase

```mach
pub fun run_steps_phase(p: *project.Project) err[outcome.Fail];
```

## fun run_dep_steps_phase

```mach
pub fun run_dep_steps_phase(p: *project.Project) err[outcome.Fail];
```

## fun run_load_phase

```mach
pub fun run_load_phase(p: *project.Project) err[outcome.Fail];
```

## fun refresh_frontend

```mach
pub fun refresh_frontend(p: *project.Project, mid: session.ModuleId, phase: FrontendPhase) res[bool, outcome.Fail];
```

re-derive a loaded project's frontend after one module's text changed, without
reloading its closure. the module is reparsed and re-walked in place; when its load
surface survived (load.reload_module), the module set and topo are as loaded and
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
pub fun load_manifest(s: *session.Session, project_root: str) res[manifest.Manifest, outcome.Fail];
```

