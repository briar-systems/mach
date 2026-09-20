# mach.lang.driver

## fun append_frontend_roots

```mach
pub fun append_frontend_roots(p: *project.Project, roots: *Vector[query.QueryKey]) err[outcome.Fail];
```

## fun run_sema_pass

```mach
pub fun run_sema_pass(p: *project.Project) err[outcome.Fail];
```

## fun run_lower_pass

```mach
pub fun run_lower_pass(p: *project.Project) err[outcome.Fail];
```

## fun run_codegen_pass

```mach
pub fun run_codegen_pass(p: *project.Project) err[outcome.Fail];
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

## fun analyze_project

```mach
pub fun analyze_project(s: *session.Session, m: *manifest.Manifest, req: *request.BuildRequest,
extra_roots: *intern.StrId, extra_root_count: u32) res[project.Project, outcome.Fail];
```

## fun analyze_project_until

```mach
pub fun analyze_project_until(s: *session.Session, m: *manifest.Manifest, req: *request.BuildRequest,
extra_roots: *intern.StrId, extra_root_count: u32, phase: FrontendPhase) res[project.Project, outcome.Fail];
```

## fun analyze_project_tolerant

```mach
pub fun analyze_project_tolerant(s: *session.Session, m: *manifest.Manifest, req: *request.BuildRequest,
extra_roots: *intern.StrId, extra_root_count: u32, phase: FrontendPhase) res[project.Project, outcome.Fail];
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
extra_roots: module fqns loaded beside the artifact's entry, nil with count 0
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

## fun run_gate_pass

```mach
pub fun run_gate_pass(p: *project.Project) res[bool, outcome.Fail];
```

## fun load_manifest

```mach
pub fun load_manifest(s: *session.Session, project_root: str) res[manifest.Manifest, outcome.Fail];
```

## fun build_project_sel

```mach
pub fun build_project_sel(s: *session.Session, project_root: str, pick: *manifest.Selection) res[project.Project, outcome.Fail];
```

## fun build_project_sel_req

```mach
pub fun build_project_sel_req(s: *session.Session, project_root: str, pick: *manifest.Selection, req: *request.BuildRequest) res[project.Project, outcome.Fail];
```

the caller's request stands in for the composed one; steps still run

## fun build_project_test

```mach
pub fun build_project_test(s: *session.Session, project_root: str, pick: *manifest.Selection) res[project.Project, outcome.Fail];
```

## fun build_project_union

```mach
pub fun build_project_union(s: *session.Session, project_root: str) res[project.Project, outcome.Fail];
```

