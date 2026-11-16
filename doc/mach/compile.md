[mach](README.md) / **compile**

# mach.compile

compiling one project for one build cell: the build, its phases, and analysis for tools

a build begins from a request and a located dependency closure, loads the selected
artifact's modules, and runs the query phases over them: resolve, sema, codegen, tests
and link. `analyze` runs the front end alone for a tool, and `refresh_frontend`
re-derives it after one module's text changes. the root manifest is read here too.

<sub>source: [`src/compile.mach`](../../src/compile.mach) · imported by 7 modules</sub>

<sub>`project` is [`mach.compile.project`](compile/project.md) · `module` is [`mach.session.module`](session/module.md) · `fail` is [`mach.base.fail`](base/fail.md) · `manifest_model` is [`mach.manifest.model`](manifest/model.md) · `manifest_select` is [`mach.manifest.select`](manifest/select.md) · `session` is [`mach.session`](session.md) · `query` is [`mach.session.query`](session/query.md) · `request` is [`mach.compile.request`](compile/request.md) · `compile_closure` is [`mach.compile.closure`](compile/closure.md) · `readout` is [`mach.report.readout`](report/readout.md) · `A` is `std.allocator` · `intern` is [`mach.base.intern`](base/intern.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`FrontendPhase`](#frontendphase) | `def` |  |
| [`Analysis`](#analysis) | `rec` | one frontend analysis for a tool: the project at a root, its request composed from the manifest and the selection as `mach check` composes it, with the modules of `files` loaded beside the root set |

| On `FrontendPhase` | |
| :--- | :--- |
| [`FRONTEND_PARSE`](#frontendphase) · [`FRONTEND_RESOLVE`](#frontendphase) · [`FRONTEND_SEMA`](#frontendphase) | |
| [`refresh_frontend`](#refresh_frontend) | re-derive a loaded project's frontend after one module's text changed, without reloading its closure |

| On `Analysis` | |
| :--- | :--- |
| [`analyze`](#analyze) | the frontend analysis tools share: the project comes back whenever the frontend ran, whatever its phases decided, so a consumer can read the trees, resolve results and sema results beside the diagnostics |

| Functions | |
| :--- | :--- |
| [`append_frontend_roots`](#append_frontend_roots) | |
| [`append_test_roots`](#append_test_roots) | the codegen roots and each test object's lowering and codegen |
| [`phase_finish`](#phase_finish) | a phase the query engine does not run is published as one it does: the project's diagnostics are refreshed beside its result |
| [`reconfigure`](#reconfigure) | a loaded project under another request from its host: the build context is frozen again, so what read the old one is computed again, and nothing reads a context no freeze captured |
| [`run_sema_pass`](#run_sema_pass) | |
| [`run_codegen_pass`](#run_codegen_pass) | the codegen stage: each emitted module and instance unit lowered and compiled, or its object restored from `obj/` |
| [`run_test_pass`](#run_test_pass) | the test stage: each test object, lowered against its module's normal object |
| [`run_link_pass`](#run_link_pass) | the link stage: the artifact `job` names, written by its action only when an input of the link moved since it last ran or the artifact on disk is not the one it wrote |
| [`closure_locate`](#closure_locate) | locate a root manifest's dependency closure with no build cell configured, as a build locates it: every dependency under dep/, its manifest and its direct edges, and nothing is checked against git or the pins |
| [`closure_checked`](#closure_checked) | locate a root manifest's dependency closure as `closure_locate` does, holding every edge to `check` as it is located, as `mach dep verify` does |
| [`begin_build`](#begin_build) | begin the build the request names: every goal, a test goal included, loads the selected artifact's closure |
| [`run_load_phase`](#run_load_phase) | |
| [`load_manifest`](#load_manifest) | the root manifest of the project at `project_root`, read once: its text, an open buffer's when the session holds one, becomes the session's input for the file, so a warm session sees an edit to it as an input change, and the manifest records the input and the revision it was read at |
| [`project_root_of`](#project_root_of) | the root of the project a file belongs to: the nearest directory above it holding a mach.toml, none when no directory does |
| [`module_under`](#module_under) | the module the file at `path` is under the project at `root`: none when it lies outside the source directory |
| [`file_module`](#file_module) | the module the file at `path` is loaded as beside the project at `root`: none when the file belongs to another project or lies outside the source directory |

---

## FrontendPhase

```mach
pub def FrontendPhase: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `FRONTEND_PARSE` | `0` |  |
| `FRONTEND_RESOLVE` | `1` |  |
| `FRONTEND_SEMA` | `2` |  |

### refresh_frontend

```mach
pub fun refresh_frontend(p: *project.Project, mid: module.ModuleId, phase: FrontendPhase) res[bool, fail.Fail]
```

re-derive a loaded project's frontend after one module's text changed, without
reloading its closure. the module is reparsed and re-walked in place; when its load
surface survived (compile_load.module_reload), the module set and topo are as loaded and
the query phases rerun over them, so every unchanged module's resolve and sema are
hits and only the edited module and its dependents recompute. a surface that did
not survive is reported as ok(false): the caller reloads

| Parameter | |
| :--- | :--- |
| `p` | the loaded project, at least at FRONTEND_RESOLVE |
| `mid` | the module whose text changed |
| `phase` | the frontend phase to re-derive to, at most the one the project was loaded to |
| **returns** | ok(true) when refreshed, a rejection included since the project's standing carries it; ok(false) when the caller must reload; or the internal failure |

## Analysis

```mach
pub rec Analysis {
    root:       str;
    manifest:   *manifest_model.Manifest;
    selection:  *manifest_select.Selectors;
    roots:      project.RootSet;
    files:      *str;
    file_count: u32;
    phase:      FrontendPhase;
}
```

one frontend analysis for a tool: the project at a root, its request composed from the
manifest and the selection as `mach check` composes it, with the modules of `files`
loaded beside the root set

| Field | Type | |
| :--- | :--- | :--- |
| `root` | `str` | the project's directory, the one holding its mach.toml |
| `manifest` | `*manifest_model.Manifest` | the project's manifest when the caller already read it, nil to read it from `root` |
| `selection` | `*manifest_select.Selectors` | the artifact, target and profile selection, one cell of which is analyzed |
| `roots` | `project.RootSet` | the root set the frontend loads: `project.ROOT_ARTIFACT` is the selected artifact's closure, `project.ROOT_UNION` every artifact's with the modules only a foreign target reaches gated out and `{artifact.<id>.out}` resolving over every artifact some artifact needs |
| `files` | `*str` | source files whose modules load beside the root set, nil with count 0; a file of another project, or outside the root's source directory, is skipped |
| `file_count` | `u32` | how many files |
| `phase` | `FrontendPhase` | the last frontend phase to run |

### analyze

```mach
pub fun analyze(s: *session.Session, an: *Analysis) res[project.Project, fail.Fail]
```

the frontend analysis tools share: the project comes back whenever the frontend ran,
whatever its phases decided, so a consumer can read the trees, resolve results and sema
results beside the diagnostics. every module that reached a phase carries that phase's
product on its ModuleEntry even when the module's own query was rejected; the standing is
frontend.frontend_status(p). a rejected phase ends the analysis at that phase; err is an
operational failure only (an unreadable manifest, a selection naming no cell, allocation,
I/O). every string `an` holds is borrowed for the call, and the project borrows `root`

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `an` | what to analyze |
| **returns** | the project, released by the caller with project.dnit_project |

## Functions

### append_frontend_roots

```mach
pub fun append_frontend_roots(p: *project.Project, roots: *Vector[query.QueryKey]) err[fail.Fail]
```

### append_test_roots

```mach
pub fun append_test_roots(p: *project.Project, roots: *Vector[query.QueryKey]) err[fail.Fail]
```

the codegen roots and each test object's lowering and codegen

### phase_finish

```mach
pub fun phase_finish(p: *project.Project,
    result: err[fail.Fail]) err[fail.Fail]
```

a phase the query engine does not run is published as one it does: the
project's diagnostics are refreshed beside its result

### reconfigure

```mach
pub fun reconfigure(p: *project.Project, req: request.BuildRequest) err[fail.Fail]
```

a loaded project under another request from its host: the build context is frozen again, so
what read the old one is computed again, and nothing reads a context no freeze captured

### run_sema_pass

```mach
pub fun run_sema_pass(p: *project.Project) err[fail.Fail]
```

### run_codegen_pass

```mach
pub fun run_codegen_pass(p: *project.Project) err[fail.Fail]
```

the codegen stage: each emitted module and instance unit lowered and compiled, or its object
restored from `obj/`

### run_test_pass

```mach
pub fun run_test_pass(p: *project.Project) err[fail.Fail]
```

the test stage: each test object, lowered against its module's normal object

### run_link_pass

```mach
pub fun run_link_pass(p: *project.Project, job: *project.LinkJob) err[fail.Fail]
```

the link stage: the artifact `job` names, written by its action only when an input of the
link moved since it last ran or the artifact on disk is not the one it wrote. a build that
compiled nothing leaves the artifact as it is when the cell's link record says this link wrote
the bytes it holds (compile/cache)

### closure_locate

```mach
pub fun closure_locate(s: *session.Session, m: *manifest_model.Manifest, project_root: str,
    unrealized: compile_closure.Unrealized) res[project.Closure, fail.Fail]
```

locate a root manifest's dependency closure with no build cell configured, as a
build locates it: every dependency under dep/, its manifest and its direct
edges, and nothing is checked against git or the pins

| Parameter | |
| :--- | :--- |
| `s` | the session; its build allocator owns the closure |
| `m` | the root manifest |
| `project_root` | the root project's directory |
| `unrealized` | what a dependency nothing is realized for is |
| **returns** | the closure, released with `project.closure_dnit` |

### closure_checked

```mach
pub fun closure_checked(s: *session.Session, m: *manifest_model.Manifest, project_root: str,
    check: *compile_closure.Check) res[project.Closure, fail.Fail]
```

locate a root manifest's dependency closure as `closure_locate` does, holding every edge
to `check` as it is located, as `mach dep verify` does

| Parameter | |
| :--- | :--- |
| `check` | what each located edge is held to |
| **returns** | the closure; err naming the first edge that does not locate or does not hold |

### begin_build

```mach
pub fun begin_build(s: *session.Session, m: *manifest_model.Manifest, closure: *project.Closure, req: *request.BuildRequest,
    progress: *readout.Progress) res[project.Project, fail.Fail]
```

begin the build the request names: every goal, a test goal included, loads the
selected artifact's closure

| Parameter | |
| :--- | :--- |
| `m` | the root manifest the cell was planned from |
| `closure` | `m`'s closure as the plan located it, which the project borrows |

### run_load_phase

```mach
pub fun run_load_phase(p: *project.Project) err[fail.Fail]
```

### load_manifest

```mach
pub fun load_manifest(s: *session.Session, project_root: str) res[manifest_model.Manifest, fail.Fail]
```

the root manifest of the project at `project_root`, read once: its text, an
open buffer's when the session holds one, becomes the session's input for the
file, so a warm session sees an edit to it as an input change, and the manifest
records the input and the revision it was read at

| Parameter | |
| :--- | :--- |
| `s` | the session; its build allocator owns the manifest |
| `project_root` | the project root |
| **returns** | the manifest, released with `manifest_model.dnit`; err when it cannot be read, does not parse or fails a check |

### project_root_of

```mach
pub fun project_root_of(alloc: *A.Allocator, file_path: str) res[opt[str], fail.Fail]
```

the root of the project a file belongs to: the nearest directory above it holding a
mach.toml, none when no directory does

### module_under

```mach
pub fun module_under(alloc: *A.Allocator, itn: *intern.Interner, path: str, root: str, id_text: str,
    src_text: str) res[opt[intern.StrId], fail.Fail]
```

the module the file at `path` is under the project at `root`: none when it lies outside the
source directory

| Parameter | |
| :--- | :--- |
| `id_text` | the project id, the module path's first segment |
| `src_text` | the manifest's source directory, relative to `root` |

### file_module

```mach
pub fun file_module(alloc: *A.Allocator, itn: *intern.Interner, path: str, root: str, id_text: str,
    src_text: str) res[opt[intern.StrId], fail.Fail]
```

the module the file at `path` is loaded as beside the project at `root`: none when the
file belongs to another project or lies outside the source directory

| Parameter | |
| :--- | :--- |
| `id_text` | the project id, the module path's first segment |
| `src_text` | the manifest's source directory, relative to `root` |

