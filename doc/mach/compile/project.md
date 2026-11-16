[mach](../README.md) / [compile](README.md) / **project**

# mach.compile.project

one project compiled for one build cell: its configuration, target, modules and objects

a `Project` holds the module table with each module's staged products and cache record,
the instance units, the test objects and the link job. a `Closure` is the located
dependency closure a plan's cells share.

<sub>source: [`src/compile/project.mach`](../../../src/compile/project.mach) · imported by 27 modules</sub>

<sub>`intern` is [`mach.base.intern`](../base/intern.md) · `manifest_model` is [`mach.manifest.model`](../manifest/model.md) · `base_source` is [`mach.base.source`](../base/source.md) · `manifest_link` is [`mach.manifest.link`](../manifest/link.md) · `manifest_plan` is [`mach.manifest.plan`](../manifest/plan.md) · `A` is `std.allocator` · `template` is [`mach.manifest.template`](../manifest/template.md) · `lower_context` is [`mach.lower.context`](../lower/context.md) · `cache_record` is [`mach.compile.cache.record`](cache/record.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `module` is [`mach.session.module`](../session/module.md) · `front_load` is [`mach.front.load`](../front/load.md) · `comptime_scope` is [`mach.front.comptime.scope`](../front/comptime/scope.md) · `expect` is [`mach.front.expect`](../front/expect.md) · `collections_vector` is `std.collections.vector` · `session` is [`mach.session`](../session.md) · `request` is [`mach.compile.request`](request.md) · `query` is [`mach.session.query`](../session/query.md) · `resolved` is [`mach.target.resolved`](../target/resolved.md) · `backend` is [`mach.compile.backend`](backend.md) · `compiler` is [`mach.compile.cache.compiler`](cache/compiler.md) · `handle` is [`mach.base.handle`](../base/handle.md) · `disk` is [`mach.base.source.provider.disk`](../base/source/provider/disk.md) · `overlay` is [`mach.base.source.provider.overlay`](../base/source/provider/overlay.md) · `demand` is [`mach.compile.demand`](demand.md) · `cache_instances` is [`mach.compile.cache.instances`](cache/instances.md) · `cache_objects` is [`mach.compile.cache.objects`](cache/objects.md) · `linkage` is [`mach.lower.linkage`](../lower/linkage.md) · `readout` is [`mach.report.readout`](../report/readout.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`RootSet`](#rootset) | `def` |  |
| [`TargetEntry`](#targetentry) | `rec` |  |
| [`DepEntry`](#depentry) | `rec` |  |
| [`Closure`](#closure) | `rec` | a root manifest's located dependency closure: every dependency realized under dep/, its manifest read once, and its direct edges |
| [`ProjectConfig`](#projectconfig) | `rec` | what a configured build cell compiles and where it writes |
| [`ArtifactEntry`](#artifactentry) | `rec` | an artifact entry the union walk loads: the entry module, the artifact it is of and where the artifact's `entry` is written in the manifest |
| [`ModuleEntry`](#moduleentry) | `rec` | a module the project loaded: its identity, what the load decided for it, and the codegen staging of its objects |
| [`Record`](#record) | `rec` | the cache record a compiled object carries into `obj/`, nil until the cache publishes one |
| [`TestObject`](#testobject) | `rec` | a module's test object, beside its normal object and never changing it: the module's tests and what they reach that the normal object does not define |
| [`InstanceUnit`](#instanceunit) | `rec` | an instance unit the build compiles (compile/demand): its key, its origin and bucket, its name, where its step writes its text and listing, and its object and facts as its codegen product holds them, borrowed like a module's |
| [`Project`](#project) | `rec` |  |
| [`LinkJob`](#linkjob) | `rec` | the link a build asks of its link stage: the artifact it writes and the action that writes it from the stage's current objects |

| On `RootSet` | |
| :--- | :--- |
| [`ROOT_ARTIFACT`](#rootset) · [`ROOT_UNION`](#rootset) | |

| On `TargetEntry` | |
| :--- | :--- |
| [`target_entry_of`](#target_entry_of) | the target entry a selected build unit describes: the one place a TargetEntry is built, so every field is carried wherever an entry is made |

| On `DepEntry` | |
| :--- | :--- |
| [`dep_entries_dnit`](#dep_entries_dnit) | release `count` dependency entries and their array of `capacity`; a nil array owns nothing |

| On `Closure` | |
| :--- | :--- |
| [`closure_init`](#closure_init) | a closure that holds no dependency |
| [`closure_dnit`](#closure_dnit) | |

| On `ModuleEntry` | |
| :--- | :--- |
| [`recorded_dnit`](#recorded_dnit) | give back the front end's warnings a module's record held |
| [`lower_scope_dnit`](#lower_scope_dnit) | release the scope lowering bound a module's constants in |
| [`dep_count`](#dep_count) | how many modules a loaded module imports |

| On `Project` | |
| :--- | :--- |
| [`record_dnit`](#record_dnit) | give back a record's bytes |
| [`link_name_for`](#link_name_for) | the name an `ext` symbol links by: its source name, which the object format's writer spells as the format names a C symbol the name a C symbol `bare` takes in the target's objects: the prefix mink's registry gives a C symbol of the target's format on its architecture, then `bare` |
| [`staged_dnit`](#staged_dnit) | a staged object is the project's until a query takes it, and `object` is borrowed |
| [`staged_test_dnit`](#staged_test_dnit) · [`all_staged_dnit`](#all_staged_dnit) | |
| [`fresh_staged_dnit`](#fresh_staged_dnit) | a restored product stays staged across operations: it is keyed by the key it was read under and the next load discards it if that changed |
| [`units_clear`](#units_clear) | forget the instance units, giving back the paths, staged objects and records the project owns of them; their objects are their codegen products' |
| [`dnit_project`](#dnit_project) | release a project and everything it owns; a nil project holds nothing |
| [`set_origin`](#set_origin) | attribute what the project's phases report directly, outside a query, to the phase now running: both the project's store and the session's |
| [`init_project`](#init_project) | |
| [`module_named`](#module_named) | the module loaded under `fqn`, nil when none is |
| [`module_count`](#module_count) | the loaded module count; the next ModuleId to be issued |
| [`module_at`](#module_at) | the entry behind a ModuleId the project has issued |
| [`module_reserve`](#module_reserve) | make room for one more entry so the next module_append cannot fail |
| [`module_append`](#module_append) | publish an entry under the next ModuleId into the slot module_reserve made |
| [`dep_at`](#dep_at) | the module a loaded module imports at `i`, as its load record names it |
| [`module_by_fqn`](#module_by_fqn) | the entry loaded under a fully qualified name |
| [`module_by_file`](#module_by_file) | the loaded module a source file backs, by a scan of the module table: editor edits arrive as file ids and the table is small beside the work a refresh saves |

---

## RootSet

```mach
pub def RootSet: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `ROOT_ARTIFACT` | `0` |  |
| `ROOT_UNION` | `1` |  |

## TargetEntry

```mach
pub rec TargetEntry {
    name:          intern.StrId;
    os_name:       intern.StrId;
    isa_name:      intern.StrId;
    abi_name:      intern.StrId;
    of_name:       intern.StrId;
    platform_name: intern.StrId;
    # the declared target the entry selects, with every key the manifest states
    declared:   manifest_model.TargetDef;
    entrypoint: intern.StrId;
    # where the artifact's `entry` is written in the manifest
    entry_at: base_source.Location;
    # the artifact's kind, as mink's output kinds spell it
    kind: Spelled;
    # the name of the artifact's `linkage`, nil for the target's default, and where it was
    # written
    linkage:      intern.StrId;
    linkage_from: manifest_model.Origin;
    # the selected profile's `optimize`
    optimize:  bool;
    libs:      *manifest_link.LinkRequirement;
    lib_count: u32;
}
```

### target_entry_of

```mach
pub fun target_entry_of(bu: *manifest_plan.BuildUnit) TargetEntry
```

the target entry a selected build unit describes: the one place a TargetEntry is
built, so every field is carried wherever an entry is made

## DepEntry

```mach
pub rec DepEntry {
    alias:           intern.StrId;
    project_id:      intern.StrId;
    project_dir_src: intern.StrId;
    vendor_root:     intern.StrId;
    source:          intern.StrId;
    ref:             intern.StrId;
    chain:           intern.StrId;
    root_declared:   bool;
    # the project each of its dependency declarations reaches, nil for one
    # left out as never realized
    direct:       *intern.StrId;
    direct_count: u32;
    manifest:     manifest_model.Manifest;
}
```

### dep_entries_dnit

```mach
pub fun dep_entries_dnit(alloc: *A.Allocator, deps: *DepEntry, count: u32, capacity: usize)
```

release `count` dependency entries and their array of `capacity`; a nil array owns
nothing

## Closure

```mach
pub rec Closure {
    alloc: *A.Allocator;
    deps:  *DepEntry;
    count: u32;
}
```

a root manifest's located dependency closure: every dependency realized under
dep/, its manifest read once, and its direct edges. a plan locates it once and
every cell it configures borrows it; a project configured without one locates
and owns its own

| Field | Type | |
| :--- | :--- | :--- |
| `alloc` | `*A.Allocator` | owns the entries, their manifests and their edges |

### closure_init

```mach
pub fun closure_init(alloc: *A.Allocator) Closure
```

a closure that holds no dependency

### closure_dnit

```mach
pub fun closure_dnit(c: *Closure)
```

## ProjectConfig

```mach
pub rec ProjectConfig {
    id:           intern.StrId;
    version:      intern.StrId;
    bin_name:     intern.StrId;
    icon:         intern.StrId;
    app_manifest: intern.StrId;
    # the cell's artifact as its manifest declares it, borrowed, nil for a cell of none
    artifact:          *manifest_model.ArtifactDef;
    module:            intern.StrId;
    dir_src:           intern.StrId;
    project_root:      intern.StrId;
    home_root:         intern.StrId;
    owner:             DepEntry;
    dependency_owned:  bool;
    closure:           Closure;
    targets:           *TargetEntry;
    target_count:      u32;
    deps:              *DepEntry;
    dep_count:         u32;
    cascade_libs:      *manifest_link.LinkRequirement;
    cascade_lib_count: u32;
    profile:           intern.StrId;
    project_work:      intern.StrId;
    steps:             *manifest_model.StepDef;
    step_count:        u32;
    step_order:        *u32;
    step_order_count:  u32;
    art_reqs:          *template.Requirement;
    art_req_count:     u32;
    req_scopes:        *template.Requirements;
    req_scope_count:   u32;
}
```

what a configured build cell compiles and where it writes

| Field | Type | |
| :--- | :--- | :--- |
| `project_root` | `intern.StrId` | the directory of the project that declares the cell: the root project, or for a dependency's cell the dependency's realization |
| `home_root` | `intern.StrId` | the root project's directory; `project_work` is rooted here |
| `owner` | `DepEntry` | the dependency that declares the cell, as the closure holds it; meaningful only with `dependency_owned` |
| `dependency_owned` | `bool` | the cell is a requirement of a dependency's default library artifacts, built for the root project |
| `closure` | `Closure` | the closure this project located itself, when it was configured without one to borrow; empty otherwise |
| `deps` | `*DepEntry` | the realized closure the cell compiles against: the root's whole closure, or a dependency cell's own transitive closure. the array is the project's, each entry borrowed from the closure |
| `art_reqs` | `*template.Requirement` | the requirements of the cell's artifact, in its declaring manifest |
| `req_scopes` | `*template.Requirements` | one requirement scope per project the cell compiles, the declaring project's first |

## ArtifactEntry

```mach
pub rec ArtifactEntry {
    fqn:      intern.StrId;
    artifact: intern.StrId;
    at:       base_source.Location;
}
```

an artifact entry the union walk loads: the entry module, the artifact it is of and
where the artifact's `entry` is written in the manifest

## ModuleEntry

```mach
pub rec ModuleEntry {
    fqn:     intern.StrId;
    file_id: base_source.FileId;
    unit:    *lower_context.Unit;
    object:  *Object;
    # where its step writes mirl's text of the module, "" for nowhere
    ir_output: str;
    # where its step writes masc's listing of the module's object, "" for nowhere
    asm_output:    str;
    staged:        *Object;
    staged_facts:  cache_record.Facts;
    record:        Record;
    cache_checked: bool;
    cache_hit:     bool;
    cache_under:   [32]u8;
    # the key this load built for the module's object, when it has one. the key
    # leaves the module's tests out, so the digest of its whole source keys its
    # test object and says whether the object's recorded warnings still stand
    cache_key:     [32]u8;
    cache_keyed:   bool;
    source_digest: [32]u8;
    # what the module's readers' keys read of it, beside cache_key (compile/cache, key_module):
    # surface_digest its importers', template_digest an instance's of its declaring module.
    # placed_digest is the digest of where the module's file puts its template text, which says
    # whether places recorded in the file still stand
    surface_digest:  [32]u8;
    template_digest: [32]u8;
    placed_digest:   [32]u8;
    # the module's object is reused, its record stands for its whole source, and nothing the
    # build compiles imports it, so the build does not ask for its front end: its record stands
    # in for its demand and its front end's warnings, which `recorded` holds. something that must
    # be rebuilt from its typed form can still have it typed
    frontend_skipped: bool;
    recorded:         diagnostic.DiagnosticStore;
    recorded_held:    bool;
    stable_id:        module.StableModuleId;
    # what the load decided for the module and the imports it records
    load: front_load.Module;
    # the scope lowering binds the module's constants in, once a lowering made it
    lower_scope:  comptime_scope.ComptimeCtx;
    lower_scoped: bool;
    # where the module's `#[expect]` declarations stand, read once per parse of it
    expects: expect.Index;
    test:    TestObject;
    # its front-end products were given back once it was lowered, and its expectations,
    # collected then, are what a later refresh reads of it
    released:     bool;
    held_expects: collections_vector.Vector[diagnostic.Expectation];
}
```

a module the project loaded: its identity, what the load decided for it, and the codegen
staging of its objects. its front-end products are the query engine's, keyed by its file and
its stable id, and read through mach.compile.frontend

### recorded_dnit

```mach
pub fun recorded_dnit(m: *ModuleEntry)
```

give back the front end's warnings a module's record held

### lower_scope_dnit

```mach
pub fun lower_scope_dnit(m: *ModuleEntry)
```

release the scope lowering bound a module's constants in

### dep_count

```mach
pub fun dep_count(m: *ModuleEntry) u32
```

how many modules a loaded module imports

## Record

```mach
pub rec Record {
    bytes: *u8;
    len:   u32;
}
```

the cache record a compiled object carries into `obj/`, nil until the cache publishes one

## TestObject

```mach
pub rec TestObject {
    unit:          *lower_context.Unit;
    object:        *Object;
    staged:        *Object;
    staged_facts:  cache_record.Facts;
    record:        Record;
    cache_checked: bool;
    cache_hit:     bool;
    cache_under:   [32]u8;
}
```

a module's test object, beside its normal object and never changing it: the
module's tests and what they reach that the normal object does not define. the
fields mirror the normal object's on ModuleEntry

## InstanceUnit

```mach
pub rec InstanceUnit {
    key:        u64;
    origin:     module.ModuleId;
    bucket:     u32;
    name:       intern.StrId;
    ir_output:  str;
    asm_output: str;
    unit:       *lower_context.Unit;
    object:     *Object;
    # the key this load built for the unit's object, when every module its members read is
    # keyed, and the object `obj/` held under it, staged for the unit's codegen product
    object_key:   [32]u8;
    keyed:        bool;
    hit:          bool;
    staged:       *Object;
    staged_facts: cache_record.Facts;
    record:       Record;
}
```

an instance unit the build compiles (compile/demand): its key, its origin and bucket, its name,
where its step writes its text and listing, and its object and facts as its codegen product
holds them, borrowed like a module's

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | `<origin>#instances.<bucket>`, which reports and readout rows name it by |
| `ir_output` | `str` | where its step writes mirl's text of it, nil for nowhere; owned by the project |
| `asm_output` | `str` | where its step writes masc's listing of it, nil for nowhere; owned by the project |

## Project

```mach
pub rec Project {
    s:            *session.Session;
    alloc:        *A.Allocator;
    req:          request.BuildRequest;
    queries:      query.QueryRuntime[Project];
    diags:        diagnostic.DiagnosticStore;
    config:       ProjectConfig;
    target_entry: *TargetEntry;
    target:       resolved.Target;
    # mirl's target for the cell and mink's formats, which compile and link the build
    back: backend.Back;

    union_entries:     *ArtifactEntry;
    union_entry_count: u32;

    # the entries of artifacts this target does not build, loaded so the modules only they
    # reach are known and left out of the compiled set
    foreign_entries:     *ArtifactEntry;
    foreign_entry_count: u32;

    compiler_name: intern.StrId;
    compiler_ver:  intern.StrId;
    # the modules are keyed and `obj/` is read under those keys, until the next load
    cache_ready: bool;
    # the configuration digest every key this load built shares
    cache_config:                  [32]u8;
    compiler_identity:             compiler.Identity;
    compiler_identity_initialized: bool;
    compiler_identity_available:   bool;
    cache_hits:                    u32;
    cache_misses:                  u32;
    # the build cell's obj directory, borrowed from its plan: where each
    # module's object is written and where the object cache reads it back
    object_dir: str;
    # the cell's compiler-only state directory, `<out>/.cache`, borrowed from
    # its plan: where the object cache keeps its digest memo
    cache_dir: str;
    # the cell's expanded `[project].work`, borrowed from its plan: the build
    # owns the layout beneath it
    out_dir: str;

    modules: handle.StableChunks[ModuleEntry];
    # the front end's load over the cell's packages, read through the session's open buffers
    loader:  front_load.Loader;
    disk:    disk.Disk;
    overlay: overlay.Overlay;

    topo:     *module.ModuleId;
    topo_len: u32;
    topo_cap: u32;

    lower_visit_gen:   *u32;
    lower_visit_cap:   u32;
    lower_visit_epoch: u32;

    emit_len: u32;
    # the instance units the build compiles, ordered by origin name and bucket, the link's order
    # after the modules; set by each codegen operation from the build's instance demand
    units: collections_vector.Vector[InstanceUnit];
    # every instance the build's demand met, described for its typing
    instances: demand.Registry;
    # the cell's instance records as this load read and judged them (compile/cache)
    instance_records: cache_instances.Store;
    # the digests of the objects the cell's builds wrote into `obj/` (compile/cache/objects)
    object_digests: cache_objects.Store;
    # the cell's ir and asm directories, borrowed from its plan, where an instance unit's text
    # and listing go when the build emits them
    ir_dir:  str;
    asm_dir: str;

    roots: RootSet;

    # what the load declares about linkage and the link phase offers the linker
    linkage: linkage.Linkage;

    # the unit's phase rows, nil when nothing reads them
    progress: *readout.Progress;

    since:          u64;
    context_digest: [32]u8;
    context_frozen: bool;

    # what the link stage writes and how, set by the build for the span of its link stage
    link: *LinkJob;
    # the digest of the link configuration this build published, which the link's key reads,
    # and whether the link takes an input found by name, whose file no key covers
    link_config:       [32]u8;
    link_config_keyed: bool;
    link_by_name:      bool;
}
```

| Field | Type | |
| :--- | :--- | :--- |
| `since` | `u64` | the first query operation this project ran; a product read between its operations counts only when one of them reached it |
| `context_digest` | `[32]u8` | the fingerprint of the build context, once frozen |
| `context_frozen` | `bool` | the build context is frozen, so a change to it is refused |

### record_dnit

```mach
pub fun record_dnit(p: *Project, r: *Record)
```

give back a record's bytes

### link_name_for

```mach
pub fun link_name_for(p: *Project, bare: intern.StrId) res[intern.StrId, fail.Fail]
```

the name an `ext` symbol links by: its source name, which the object format's
writer spells as the format names a C symbol
the name a C symbol `bare` takes in the target's objects: the prefix mink's
registry gives a C symbol of the target's format on its architecture, then `bare`

### staged_dnit

```mach
pub fun staged_dnit(p: *Project, m: *ModuleEntry)
```

a staged object is the project's until a query takes it, and `object` is borrowed

### staged_test_dnit

```mach
pub fun staged_test_dnit(p: *Project, m: *ModuleEntry)
```

### all_staged_dnit

```mach
pub fun all_staged_dnit(p: *Project)
```

### fresh_staged_dnit

```mach
pub fun fresh_staged_dnit(p: *Project)
```

a restored product stays staged across operations: it is keyed by the key
it was read under and the next load discards it if that changed. a freshly
generated image staged by a codegen worker belongs to the operation that
generated it and leaves with it

### units_clear

```mach
pub fun units_clear(p: *Project)
```

forget the instance units, giving back the paths, staged objects and records the project owns
of them; their objects are their codegen products'

### dnit_project

```mach
pub fun dnit_project(p: *Project)
```

release a project and everything it owns; a nil project holds nothing

### set_origin

```mach
pub fun set_origin(p: *Project, o: diagnostic.Origin)
```

attribute what the project's phases report directly, outside a query, to the
phase now running: both the project's store and the session's

### init_project

```mach
pub fun init_project(p: *Project, s: *session.Session)
```

### module_named

```mach
pub fun module_named(p: *Project, fqn: str) module.ModuleId
```

the module loaded under `fqn`, nil when none is

### module_count

```mach
pub fun module_count(p: *Project) u32
```

the loaded module count; the next ModuleId to be issued

### module_at

```mach
pub fun module_at(p: *Project, mid: u32) *ModuleEntry
```

the entry behind a ModuleId the project has issued

the store is a handle.StableChunks: an entry is written once into a fixed
chunk and never moves, so the pointer stays valid until dnit_project however
many modules load after it. total: an id the project never issued stops the
compiler

### module_reserve

```mach
pub fun module_reserve(p: *Project) err[fail.Fail]
```

make room for one more entry so the next module_append cannot fail

### module_append

```mach
pub fun module_append(p: *Project, m: ModuleEntry) module.ModuleId
```

publish an entry under the next ModuleId into the slot module_reserve made.
total: a caller that reserved nothing first stops the compiler

### dep_at

```mach
pub fun dep_at(p: *Project, m: *ModuleEntry, i: u32) module.ModuleId
```

the module a loaded module imports at `i`, as its load record names it

### module_by_fqn

```mach
pub fun module_by_fqn(p: *Project, fqn: intern.StrId) opt[*ModuleEntry]
```

the entry loaded under a fully qualified name

the returned `*ModuleEntry` is stable until dnit_project: the store never
moves an entry when more modules load, so a holder may keep it across a
nested load and read the same entry afterwards

### module_by_file

```mach
pub fun module_by_file(p: *Project, fid: base_source.FileId) opt[module.ModuleId]
```

the loaded module a source file backs, by a scan of the module table: editor edits
arrive as file ids and the table is small beside the work a refresh saves

| Parameter | |
| :--- | :--- |
| `p` | the project |
| `fid` | the file |
| **returns** | the module's id, or none when no loaded module reads that file |

## LinkJob

```mach
pub rec LinkJob {
    artifact: str;
    ctx:      ptr;
    run:      fun(ptr, *Project, *diagnostic.DiagnosticStore) res[backend.LinkRead, fail.Fail];
}
```

the link a build asks of its link stage: the artifact it writes and the action that writes it
from the stage's current objects. the stage decides whether it runs

| Field | Type | |
| :--- | :--- | :--- |
| `artifact` | `str` | the path the action writes, whose bytes the stage records |
| `ctx` | `ptr` | the build's state the action reads |
| `run` | `fun(ptr, *Project, *diagnostic.DiagnosticStore) res[backend.LinkRead, fail.Fail]` | writes `artifact`, reporting what the link tells of itself to the store it is given, answering what the link read |

