[mach](../README.md) / [compile](README.md) / **project**

# mach.compile.project

one project compiled for one build cell: its configuration, target, modules and objects

a `Project` holds the module table with each module's staged products and cache record,
the test objects and the link job. a `Closure` is the located
dependency closure a plan's cells share.

Source: [`src/compile/project.mach`](../../../src/compile/project.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md), [`mach.base.fail`](../base/fail.md), [`mach.base.handle`](../base/handle.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.base.source.provider.disk`](../base/source/provider/disk.md), [`mach.base.source.provider.overlay`](../base/source/provider/overlay.md), [`mach.compile.backend`](backend.md), [`mach.compile.cache.compiler`](cache/compiler.md), [`mach.compile.cache.objects`](cache/objects.md), [`mach.compile.cache.record`](cache/record.md), [`mach.compile.demand`](demand.md), [`mach.compile.readahead`](readahead.md), [`mach.compile.request`](request.md), [`mach.front.comptime.scope`](../front/comptime/scope.md), [`mach.front.expect`](../front/expect.md), [`mach.front.load`](../front/load.md), [`mach.lower.context`](../lower/context.md), [`mach.lower.linkage`](../lower/linkage.md), [`mach.manifest.link`](../manifest/link.md), [`mach.manifest.model`](../manifest/model.md), [`mach.manifest.plan`](../manifest/plan.md), [`mach.manifest.template`](../manifest/template.md), [`mach.report.readout`](../report/readout.md), [`mach.session`](../session.md), [`mach.session.module`](../session/module.md), [`mach.session.query`](../session/query.md), [`mach.target.resolved`](../target/resolved.md), [`mach.type`](../type.md).

| Type | | |
| :--- | :--- | :--- |
| [`RootSet`](#rootset) | `def` |  |
| [`TargetEntry`](#targetentry) | `rec` | the target a configured cell builds for and the artifact it builds, as target_entry_of makes it |
| [`DepEntry`](#depentry) | `rec` | one dependency of a located closure |
| [`Closure`](#closure) | `rec` | a root manifest's located dependency closure: every dependency realized under dep/, its manifest read once, and its direct edges |
| [`ProjectConfig`](#projectconfig) | `rec` | what a configured build cell compiles and where it writes |
| [`ArtifactEntry`](#artifactentry) | `rec` | an artifact entry the union walk loads: the entry module, the artifact it is of and where the artifact's `entry` is written in the manifest |
| [`ModuleEntry`](#moduleentry) | `rec` | a module the project loaded: its identity, what the load decided for it, and the codegen staging of its objects |
| [`Record`](#record) | `rec` | the cache record a compiled object carries into `obj/`, nil until the cache publishes one |
| [`TestObject`](#testobject) | `rec` | a module's test object, beside its normal object and never changing it: the module's tests and what they reach that the normal object does not define |
| [`Project`](#project) | `rec` | one project compiled for one build cell: its request, configuration and target, its module table with each module's products and objects, the object cache's state and the link job |
| [`LinkJob`](#linkjob) | `rec` | the link a build asks of its link stage: the artifact it writes and the action that writes it from the stage's current objects |

| On `RootSet` | |
| :--- | :--- |
| [`ROOT_ARTIFACT`](#rootset), [`ROOT_UNION`](#rootset) | |

| On `TargetEntry` | |
| :--- | :--- |
| [`target_entry_of`](#target_entry_of) | the target entry a selected build unit describes: the one place a TargetEntry is built, so every field is carried wherever an entry is made |

| On `DepEntry` | |
| :--- | :--- |
| [`dep_entries_dnit`](#dep_entries_dnit) | release `count` dependency entries and their array of `capacity`; a nil array owns nothing |

| On `Closure` | |
| :--- | :--- |
| [`closure_init`](#closure_init) | a closure that holds no dependency |
| [`closure_dnit`](#closure_dnit) | free every entry of a closure, leaving it empty |

| On `ModuleEntry` | |
| :--- | :--- |
| [`recorded_dnit`](#recorded_dnit) | give back the front end's warnings a module's record held |
| [`lower_scope_dnit`](#lower_scope_dnit) | release the scope lowering bound a module's constants in |
| [`dep_count`](#dep_count) | how many modules a loaded module imports |

| On `Project` | |
| :--- | :--- |
| [`record_dnit`](#record_dnit) | give back a record's bytes |
| [`unit_count`](#unit_count) | the units the build compiles its emitted modules in, one object each: one per module, or one for them all where the family that compiles the build takes an artifact whole |
| [`unit_module`](#unit_module) | the module whose entry holds unit `u`'s object, mirl text and listing: the module itself, or for an artifact taken whole the last module emitted, which no other emitted module imports |
| [`link_name_for`](#link_name_for) | the name a C symbol `bare` takes in the target's objects: the prefix mink's registry gives a C symbol of the target's format on its architecture, then `bare` |
| [`staged_dnit`](#staged_dnit) | a staged object is the project's until a query takes it, and `object` is borrowed |
| [`staged_test_dnit`](#staged_test_dnit) | a staged test object is the project's until a query takes it, and the test `object` is borrowed |
| [`all_staged_dnit`](#all_staged_dnit) | give back every module's staged object and staged test object |
| [`fresh_staged_dnit`](#fresh_staged_dnit) | a restored product stays staged across operations: it is keyed by the key it was read under and the next load discards it if that changed |
| [`dnit_project`](#dnit_project) | release a project and everything it owns; a nil project holds nothing |
| [`set_origin`](#set_origin) | attribute what the project's phases report directly, outside a query, to the phase now running: both the project's store and the session's |
| [`init_project`](#init_project) | make `p` an empty project of session `s`, resetting the session's diagnostics and presentation |
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
    name:          mach.base.intern.StrId;
    os_name:       mach.base.intern.StrId;
    isa_name:      mach.base.intern.StrId;
    abi_name:      mach.base.intern.StrId;
    of_name:       mach.base.intern.StrId;
    platform_name: mach.base.intern.StrId;
    # the declared target the entry selects, with every key the manifest states
    declared:   mach.manifest.model.TargetDef;
    entrypoint: mach.base.intern.StrId;
    # where the artifact's `entry` is written in the manifest
    entry_at: mach.base.source.Location;
    # the artifact's kind, as mink's output kinds spell it
    kind: Spelled;
    # the name of the artifact's `linkage`, nil for the target's default, and where it was
    # written
    linkage:      mach.base.intern.StrId;
    linkage_from: mach.manifest.model.Origin;
    # the selected profile's `optimize`
    optimize:  bool;
    libs:      *mach.manifest.link.LinkRequirement;
    lib_count: u32;
}
```

the target a configured cell builds for and the artifact it builds, as target_entry_of makes it

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.intern.StrId` | the target's name |
| `os_name` | `mach.base.intern.StrId` | its system |
| `isa_name` | `mach.base.intern.StrId` | its architecture |
| `abi_name` | `mach.base.intern.StrId` | its abi |
| `of_name` | `mach.base.intern.StrId` | its object format |
| `platform_name` | `mach.base.intern.StrId` | its platform |
| `entrypoint` | `mach.base.intern.StrId` | the artifact's entry, as the manifest writes it |
| `libs` | `*mach.manifest.link.LinkRequirement` | the cell's own link requirements |
| `lib_count` | `u32` | length of `libs` |

### target_entry_of

```mach
pub fun target_entry_of(bu: *mach.manifest.plan.BuildUnit) TargetEntry
```

the target entry a selected build unit describes: the one place a TargetEntry is
built, so every field is carried wherever an entry is made

## DepEntry

```mach
pub rec DepEntry {
    alias:           mach.base.intern.StrId;
    project_id:      mach.base.intern.StrId;
    project_dir_src: mach.base.intern.StrId;
    vendor_root:     mach.base.intern.StrId;
    source:          mach.base.intern.StrId;
    ref:             mach.base.intern.StrId;
    chain:           mach.base.intern.StrId;
    root_declared:   bool;
    # the project each of its dependency declarations reaches, nil for one
    # left out as never realized
    direct:       *mach.base.intern.StrId;
    direct_count: u32;
    manifest:     mach.manifest.model.Manifest;
}
```

one dependency of a located closure

| Field | Type | |
| :--- | :--- | :--- |
| `alias` | `mach.base.intern.StrId` | the key it is declared and realized under |
| `project_id` | `mach.base.intern.StrId` | the project id its manifest states |
| `project_dir_src` | `mach.base.intern.StrId` | its manifest's `src` |
| `vendor_root` | `mach.base.intern.StrId` | the directory it is realized in |
| `source` | `mach.base.intern.StrId` | a label of where it comes from, as messages show it |
| `ref` | `mach.base.intern.StrId` | the ref its selecting declaration names |
| `chain` | `mach.base.intern.StrId` | how the root reaches it, as messages show it |
| `root_declared` | `bool` | the root manifest declares it itself |
| `manifest` | `mach.manifest.model.Manifest` | its manifest, read once |

### dep_entries_dnit

```mach
pub fun dep_entries_dnit(alloc: *std.allocator.Allocator, deps: *DepEntry, count: u32, capacity: usize)
```

release `count` dependency entries and their array of `capacity`; a nil array owns
nothing

## Closure

```mach
pub rec Closure {
    alloc: *std.allocator.Allocator;
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
| `alloc` | `*std.allocator.Allocator` | owns the entries, their manifests and their edges |

### closure_init

```mach
pub fun closure_init(alloc: *std.allocator.Allocator) Closure
```

a closure that holds no dependency

### closure_dnit

```mach
pub fun closure_dnit(c: *Closure)
```

free every entry of a closure, leaving it empty

| Parameter | |
| :--- | :--- |
| `c` | closure to free, through its own allocator |

## ProjectConfig

```mach
pub rec ProjectConfig {
    id:           mach.base.intern.StrId;
    version:      mach.base.intern.StrId;
    bin_name:     mach.base.intern.StrId;
    icon:         mach.base.intern.StrId;
    app_manifest: mach.base.intern.StrId;
    # the cell's artifact as its manifest declares it, borrowed, nil for a cell of none
    artifact:          *mach.manifest.model.ArtifactDef;
    module:            mach.base.intern.StrId;
    dir_src:           mach.base.intern.StrId;
    project_root:      mach.base.intern.StrId;
    home_root:         mach.base.intern.StrId;
    owner:             DepEntry;
    dependency_owned:  bool;
    closure:           Closure;
    targets:           *TargetEntry;
    target_count:      u32;
    deps:              *DepEntry;
    dep_count:         u32;
    dep_allow:         *mach.base.diagnostic.kind.KindSet;
    cascade_libs:      *mach.manifest.link.LinkRequirement;
    cascade_lib_count: u32;
    profile:           mach.base.intern.StrId;
    project_work:      mach.base.intern.StrId;
    steps:             *mach.manifest.model.StepDef;
    step_count:        u32;
    step_order:        *u32;
    step_order_count:  u32;
    art_reqs:          *mach.manifest.template.Requirement;
    art_req_count:     u32;
    req_scopes:        *mach.manifest.template.Requirements;
    req_scope_count:   u32;
}
```

what a configured build cell compiles and where it writes

| Field | Type | |
| :--- | :--- | :--- |
| `project_root` | `mach.base.intern.StrId` | the directory of the project that declares the cell: the root project, or for a dependency's cell the dependency's realization |
| `home_root` | `mach.base.intern.StrId` | the root project's directory; `project_work` is rooted here |
| `owner` | `DepEntry` | the dependency that declares the cell, as the closure holds it; meaningful only with `dependency_owned` |
| `dependency_owned` | `bool` | the cell is a requirement of a dependency's default library artifacts, built for the root project |
| `closure` | `Closure` | the closure this project located itself, when it was configured without one to borrow; empty otherwise |
| `deps` | `*DepEntry` | the realized closure the cell compiles against: the root's whole closure, or a dependency cell's own transitive closure. the array is the project's, each entry borrowed from the closure |
| `dep_allow` | `*mach.base.diagnostic.kind.KindSet` | the `allow` of the profile each of `deps` selects with no profile named, parallel to `deps`, nil unless the build shows dependencies' warnings |
| `art_reqs` | `*mach.manifest.template.Requirement` | the requirements of the cell's artifact, in its declaring manifest |
| `req_scopes` | `*mach.manifest.template.Requirements` | one requirement scope per project the cell compiles, the declaring project's first |

## ArtifactEntry

```mach
pub rec ArtifactEntry {
    fqn:      mach.base.intern.StrId;
    artifact: mach.base.intern.StrId;
    at:       mach.base.source.Location;
}
```

an artifact entry the union walk loads: the entry module, the artifact it is of and
where the artifact's `entry` is written in the manifest

## ModuleEntry

```mach
pub rec ModuleEntry {
    fqn:     mach.base.intern.StrId;
    file_id: mach.base.source.FileId;
    unit:    *mach.lower.context.Unit;
    object:  *Object;
    # where its object is written once its unit completes, "" for held until the build ends
    object_output: str;
    # the object went to object_output and `object` is empty until a link reads it back
    object_away: bool;
    # where its step writes mirl's text of the module, "" for nowhere
    ir_output: str;
    # where its step writes masc's listing of the module's object, "" for nowhere
    asm_output:    str;
    staged:        *Object;
    staged_facts:  mach.compile.cache.record.Facts;
    record:        Record;
    cache_checked: bool;
    cache_hit:     bool;
    cache_under:   [32]u8;
    # the key this load built for the module's object, when it has one, which covers its whole
    # source and keys its test object
    cache_key:   [32]u8;
    cache_keyed: bool;
    # what the module's readers' keys read of it, beside cache_key (compile/cache, key_module):
    # surface_digest its importers', template_digest an instance's of its declaring module
    surface_digest:  [32]u8;
    template_digest: [32]u8;
    # the module's object is reused, its record stands for its whole source, and nothing the
    # build compiles imports it, so the build does not ask for its front end: its record stands
    # in for its front end's warnings, which `recorded` holds
    frontend_skipped: bool;
    recorded:         mach.base.diagnostic.DiagnosticStore;
    recorded_held:    bool;
    stable_id:        mach.session.module.StableModuleId;
    # what the load decided for the module and the imports it records
    load: mach.front.load.Module;
    # the scope lowering binds the module's constants in, once a lowering made it
    lower_scope:  mach.front.comptime.scope.ComptimeCtx;
    lower_scoped: bool;
    # where the module's `#[expect]` declarations stand, read once per parse of it
    expects: mach.front.expect.Index;
    test:    TestObject;
    # its front-end products were given back once it was lowered, and its expectations,
    # collected then, are what a later refresh reads of it
    released:     bool;
    held_expects: std.collections.vector.Vector[mach.base.diagnostic.Expectation];
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
    unit:          *mach.lower.context.Unit;
    object:        *Object;
    object_output: str;
    object_away:   bool;
    staged:        *Object;
    staged_facts:  mach.compile.cache.record.Facts;
    record:        Record;
    cache_checked: bool;
    cache_hit:     bool;
    cache_under:   [32]u8;
}
```

a module's test object, beside its normal object and never changing it: the
module's tests and what they reach that the normal object does not define. the
fields mirror the normal object's on ModuleEntry

## Project

```mach
pub rec Project {
    s:            *mach.session.Session;
    alloc:        *std.allocator.Allocator;
    req:          mach.compile.request.BuildRequest;
    queries:      mach.session.query.QueryRuntime[Project];
    diags:        mach.base.diagnostic.DiagnosticStore;
    config:       ProjectConfig;
    target_entry: *TargetEntry;
    target:       mach.target.resolved.Target;
    # mirl's target for the cell and mink's formats, which compile and link the build
    back: mach.compile.backend.Back;

    union_entries:     *ArtifactEntry;
    union_entry_count: u32;

    # the entries of artifacts this target does not build, loaded so the modules only they
    # reach are known and left out of the compiled set
    foreign_entries:     *ArtifactEntry;
    foreign_entry_count: u32;

    compiler_name: mach.base.intern.StrId;
    compiler_ver:  mach.base.intern.StrId;
    # the modules are keyed and `obj/` is read under those keys, until the next load
    cache_ready: bool;
    # the configuration digest every key this load built shares
    cache_config:                  [32]u8;
    compiler_identity:             mach.compile.cache.compiler.Identity;
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

    modules: mach.base.handle.StableChunks[ModuleEntry];
    # the front end's load over the cell's packages, read through the session's open buffers
    loader:  mach.front.load.Loader;
    disk:    mach.base.source.provider.disk.Disk;
    overlay: mach.base.source.provider.overlay.Overlay;
    # the files the running load walk reads and parses ahead of itself, nil when it reads none
    ahead: *mach.compile.readahead.ReadAhead;
    # what a codegen pass's lowerings read of other products, gathered when the pass starts; nil outside one
    views: *mach.session.query.Views;
    # the session's type table as a codegen pass on a crew froze it, which every lowering layers its
    # own types over while the session's handle names the owner's layer; nil otherwise, when a lowering
    # writes the session's table itself
    types_frozen: *mach.type.TypeInterner;
    # how many types the owner's layer holds from merged lowerings, which is every type it holds: a
    # lowering reads the frozen table, so a type the owner made itself would be one it cannot see
    types_merged: u32;
    # the session's interner as a codegen pass on a crew froze it, which every lowering names its own
    # strings over while the session's handle names the owner's child of it; nil otherwise, when a
    # lowering names its strings in the session's interner itself
    names_frozen: *mach.base.intern.Interner;

    topo:     *mach.session.module.ModuleId;
    topo_len: u32;
    topo_cap: u32;

    lower_visit_gen:   *u32;
    lower_visit_cap:   u32;
    lower_visit_epoch: u32;

    emit_len: u32;
    # the units the codegen pass and the test pass reached in order: every unit, or those up to
    # and holding the one whose failure ended the pass, which are all its presentation names
    codegen_reached: u32;
    test_reached:    u32;
    # the threads the codegen pass compiled on, 1 when it compiled on the owner, which the test pass follows
    codegen_threads: u32;
    # the threads the test pass compiled on, 0 when it compiled on the owner
    test_threads: u32;
    # every instance the build's modules met, described for its typing
    instances: mach.compile.demand.Registry;
    # the digests of the objects the cell's builds wrote into `obj/` (compile/cache/objects)
    object_digests: mach.compile.cache.objects.Store;

    roots: RootSet;

    # what the load declares about linkage and the link phase offers the linker
    linkage: mach.lower.linkage.Linkage;

    # the unit's phase rows, nil when nothing reads them
    progress: *mach.report.readout.Progress;

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

one project compiled for one build cell: its request, configuration and target, its module
table with each module's products and objects, the object cache's state and the link job

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

### unit_count

```mach
pub fun unit_count(p: *Project) u32
```

the units the build compiles its emitted modules in, one object each: one per module, or one for
them all where the family that compiles the build takes an artifact whole

### unit_module

```mach
pub fun unit_module(p: *Project, u: u32) *ModuleEntry
```

the module whose entry holds unit `u`'s object, mirl text and listing: the module itself, or for an
artifact taken whole the last module emitted, which no other emitted module imports

### link_name_for

```mach
pub fun link_name_for(p: *Project, bare: mach.base.intern.StrId) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

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

a staged test object is the project's until a query takes it, and the test `object` is borrowed

### all_staged_dnit

```mach
pub fun all_staged_dnit(p: *Project)
```

give back every module's staged object and staged test object

### fresh_staged_dnit

```mach
pub fun fresh_staged_dnit(p: *Project)
```

a restored product stays staged across operations: it is keyed by the key
it was read under and the next load discards it if that changed. a freshly
generated image staged by a codegen worker belongs to the operation that
generated it and leaves with it

### dnit_project

```mach
pub fun dnit_project(p: *Project)
```

release a project and everything it owns; a nil project holds nothing

### set_origin

```mach
pub fun set_origin(p: *Project, o: mach.base.diagnostic.Origin)
```

attribute what the project's phases report directly, outside a query, to the
phase now running: both the project's store and the session's

### init_project

```mach
pub fun init_project(p: *Project, s: *mach.session.Session)
```

make `p` an empty project of session `s`, resetting the session's diagnostics and presentation

| Parameter | |
| :--- | :--- |
| `p` | the project to initialize, which allocates in the session's build allocator |
| `s` | the session the project builds in |

### module_named

```mach
pub fun module_named(p: *Project, fqn: str) mach.session.module.ModuleId
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
pub fun module_reserve(p: *Project) err[mach.base.fail.Fail]
```

make room for one more entry so the next module_append cannot fail

### module_append

```mach
pub fun module_append(p: *Project, m: ModuleEntry) mach.session.module.ModuleId
```

publish an entry under the next ModuleId into the slot module_reserve made.
total: a caller that reserved nothing first stops the compiler

### dep_at

```mach
pub fun dep_at(p: *Project, m: *ModuleEntry, i: u32) mach.session.module.ModuleId
```

the module a loaded module imports at `i`, as its load record names it

### module_by_fqn

```mach
pub fun module_by_fqn(p: *Project, fqn: mach.base.intern.StrId) opt[*ModuleEntry]
```

the entry loaded under a fully qualified name

the returned `*ModuleEntry` is stable until dnit_project: the store never
moves an entry when more modules load, so a holder may keep it across a
nested load and read the same entry afterwards

### module_by_file

```mach
pub fun module_by_file(p: *Project, fid: mach.base.source.FileId) opt[mach.session.module.ModuleId]
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
    run:      fun(ptr, *Project, *mach.base.diagnostic.DiagnosticStore) res[mach.compile.backend.LinkRead, mach.base.fail.Fail];
}
```

the link a build asks of its link stage: the artifact it writes and the action that writes it
from the stage's current objects. the stage decides whether it runs

| Field | Type | |
| :--- | :--- | :--- |
| `artifact` | `str` | the path the action writes, whose bytes the stage records |
| `ctx` | `ptr` | the build's state the action reads |
| `run` | `fun(ptr, *Project, *mach.base.diagnostic.DiagnosticStore) res[mach.compile.backend.LinkRead, mach.base.fail.Fail]` | writes `artifact`, reporting what the link tells of itself to the store it is given, answering what the link read |

