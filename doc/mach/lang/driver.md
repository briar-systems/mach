# mach.lang.driver

Project driver. Parses `mach.toml` (plus every dep's own `mach.toml`),
selects the active target, DFS-walks the module graph starting from the
target's entrypoint, and runs [resolve](fe/resolve.md) over every
reachable module in topological order. The DFS post-order *is* the
topological order, so no separate sort runs. During the load walk,
`$if` predicates are evaluated against an incrementally-built
[`comptime.ComptimeCtx`](fe/comptime.md#comptimectx) — target params
plus already-loaded imports' pub comptime constants plus this
module's earlier pub comptime vals — so each module sees the same
constant environment when it later runs through resolve. The public
entry is [`build_project`](#build_project), which returns a fully
populated [`Project`](#project) or an error string.

The driver is the canonical *consumer* of the
[`query`](query.md) catalog: it sets the three input queries
(`Q_PROJECT_ROOT`, `Q_TARGET`, `Q_FILE_TEXT` per file loaded) and
asks for the top-level derived artifact. The DFS / decl-walk /
resolve mechanics described below are the body of `Q_RESOLVE`'s
compute path — they run inside the query database under the
session, not as standalone driver code paths. The orchestration
shape is unchanged from before the query rewrite; what moved is the
ownership of recomputation and caching.

## Types

### `TargetEntry`

```mach
pub rec TargetEntry {
    name:       intern.StrId;
    os_name:    intern.StrId;
    isa_name:   intern.StrId;
    entrypoint: intern.StrId;
}
```

One `[targets.<name>]` entry from `mach.toml`.

| Field      | Type                                          | Description                                                |
|------------|-----------------------------------------------|------------------------------------------------------------|
| name       | [`intern.StrId`](intern.md#strid)             | Interned selector (e.g. `"linux"`); matches the value passed to [`build_project`](#build_project). |
| os_name    | [`intern.StrId`](intern.md#strid)             | Interned `os` field (e.g. `"linux"`).                      |
| isa_name   | [`intern.StrId`](intern.md#strid)             | Interned `isa` field (e.g. `"x86_64"`).                    |
| entrypoint | [`intern.StrId`](intern.md#strid)             | Interned entrypoint path, relative to [`ProjectConfig.dir_src`](#projectconfig) (e.g. `"main.mach"`). |

### `DepEntry`

```mach
pub rec DepEntry {
    alias:           intern.StrId;
    project_id:      intern.StrId;
    project_dir_src: intern.StrId;
    vendor_root:     intern.StrId;
}
```

One `[deps.<alias>]` entry, paired with the dep's own resolved project
metadata (read from the dep's `mach.toml`).

| Field           | Type                                          | Description                                            |
|-----------------|-----------------------------------------------|--------------------------------------------------------|
| alias           | [`intern.StrId`](intern.md#strid)             | Interned alias as written in `[deps.<alias>]`.         |
| project_id      | [`intern.StrId`](intern.md#strid)             | Interned `[project].id` from the dep's own `mach.toml`. |
| project_dir_src | [`intern.StrId`](intern.md#strid)             | Interned `[project].dir_src` from the dep's own `mach.toml`. |
| vendor_root     | [`intern.StrId`](intern.md#strid)             | Interned absolute filesystem path to the dep root.     |

### `ProjectConfig`

```mach
pub rec ProjectConfig {
    id:           intern.StrId;
    dir_src:      intern.StrId;
    dir_dep:      intern.StrId;
    project_root: intern.StrId;
    targets:      *TargetEntry;
    target_count: u32;
    deps:         *DepEntry;
    dep_count:    u32;
}
```

Parsed `[project]` table plus the flat target and dep arrays.

| Field        | Type                                          | Description                                                |
|--------------|-----------------------------------------------|------------------------------------------------------------|
| id           | [`intern.StrId`](intern.md#strid)             | Interned `[project].id` — root FQN segment for this project's modules. |
| dir_src      | [`intern.StrId`](intern.md#strid)             | Interned `[project].dir_src` (defaults to `"src"`).        |
| dir_dep      | [`intern.StrId`](intern.md#strid)             | Interned `[project].dir_dep` (defaults to `"dep"`).        |
| project_root | [`intern.StrId`](intern.md#strid)             | Interned absolute filesystem path of the project root.     |
| targets      | [`*TargetEntry`](#targetentry)                | Array of target entries.                                   |
| target_count | `u32`                                         | Number of entries in `targets`.                            |
| deps         | [`*DepEntry`](#depentry)                      | Array of dep entries (`nil` when no `[deps]` table).       |
| dep_count    | `u32`                                         | Number of entries in `deps`.                               |

### `PubConst`

```mach
pub rec PubConst {
    name:  intern.StrId;
    value: comptime.CTValue;
}
```

One comptime constant exported by a module to its importers — copied
into a dependent module's [`ComptimeCtx`](fe/comptime.md#comptimectx)
by [`merge_pub_consts`](#internal-helpers) when the importer walks the
`use` decl.

| Field | Type                                                | Description                                  |
|-------|-----------------------------------------------------|----------------------------------------------|
| name  | [`intern.StrId`](intern.md#strid)                   | Interned identifier.                         |
| value | [`comptime.CTValue`](fe/comptime.md#ctvalue)        | The constant value.                          |

### `ModuleEntry`

```mach
pub rec ModuleEntry {
    fqn:               intern.StrId;
    file_id:           src.FileId;
    a:                 ast.Ast;
    ctx:               comptime.ComptimeCtx;
    pub_consts:        *PubConst;
    pub_const_count:   u32;
    pub_const_cap:     u32;
    deps:              *sess.ModuleId;
    dep_count:         u32;
    dep_cap:           u32;
    resolve_result:    resolve.ResolveResult;
    resolved:          bool;
}
```

One loaded module. Indexed by [`sess.ModuleId`](session.md#moduleid)
in [`Project.modules`](#project).

| Field           | Type                                              | Description                                                |
|-----------------|---------------------------------------------------|------------------------------------------------------------|
| fqn             | [`intern.StrId`](intern.md#strid)                 | Interned fully-qualified module path (e.g. `"std.types.bool"`). |
| file_id         | [`src.FileId`](source.md#fileid)                  | FileId of the module's source in [`session.sources`](session.md#session). |
| a               | [`ast.Ast`](fe/ast.md#ast)                        | Parsed AST; owned by the entry. Freed by [`dnit_project`](#dnit_project). |
| ctx             | [`comptime.ComptimeCtx`](fe/comptime.md#comptimectx) | Per-module comptime context: target params + every loaded dep's pub consts + this module's own pub comptime vals as they're discovered. |
| pub_consts      | [`*PubConst`](#pubconst)                          | Comptime vals declared `pub` in this module, in source order. |
| pub_const_count | `u32`                                             | Number of entries in `pub_consts`.                         |
| pub_const_cap   | `u32`                                             | Allocated capacity of `pub_consts`.                        |
| deps            | [`*sess.ModuleId`](session.md#moduleid)           | ModuleIds this module directly depends on (active `use` targets, dedup'd). |
| dep_count       | `u32`                                             | Number of entries in `deps`.                               |
| dep_cap         | `u32`                                             | Allocated capacity of `deps`.                              |
| resolve_result  | [`resolve.ResolveResult`](fe/resolve.md#resolveresult) | Output of [resolve](fe/resolve.md); zero-initialized until `resolved` flips. |
| resolved        | `bool`                                            | `true` once [resolve](fe/resolve.md#resolve) has run successfully. |

### `Project`

```mach
pub rec Project {
    s:              *sess.Session;
    config:         ProjectConfig;
    target_entry:   *TargetEntry;
    target:         tgt.Target;

    compiler_name:  intern.StrId;
    compiler_ver:   intern.StrId;

    modules:        *ModuleEntry;
    module_len:     u32;
    module_cap:     u32;
    by_fqn:         map.Map[intern.StrId, sess.ModuleId];

    load_status:    *LoadStatus;
    load_cap:       u32;

    topo:           *sess.ModuleId;
    topo_len:       u32;
    topo_cap:       u32;
}
```

The build's entire module graph plus the parsed config and the
selected target.

| Field          | Type                                              | Description                                                |
|----------------|---------------------------------------------------|------------------------------------------------------------|
| s              | [`*sess.Session`](session.md#session)             | Shared session (allocator, sources, diags, interner, type interner, query database). |
| config         | [`ProjectConfig`](#projectconfig)                 | Parsed `mach.toml` plus resolved dep metadata.             |
| target_entry   | [`*TargetEntry`](#targetentry)                    | Pointer into `config.targets` — the raw config entry selected by the user. |
| target         | [`tgt.Target`](target.md#target)                  | Resolved target. Composes the active OS / ISA / ABI / OF vtables; constructed by [`target.select`](target.md#select). Downstream phases read pointer width / endianness / register classes / calling convention through this. |
| compiler_name  | [`intern.StrId`](intern.md#strid)                 | Interned compiler identifier (currently `"mach"`).         |
| compiler_ver   | [`intern.StrId`](intern.md#strid)                 | Interned compiler version (currently `"0.0.0"`).           |
| modules        | [`*ModuleEntry`](#moduleentry)                    | Flat array of every loaded module, indexed by [`ModuleId`](session.md#moduleid). |
| module_len     | `u32`                                             | Number of entries in `modules`.                            |
| module_cap     | `u32`                                             | Allocated capacity of `modules`.                           |
| by_fqn         | `map.Map[intern.StrId, sess.ModuleId]`            | FQN → ModuleId index; populated as modules are registered. |
| load_status    | `*LoadStatus`                                     | DFS load state, parallel to `modules`. See [DFS load](#dfs-load). |
| load_cap       | `u32`                                             | Allocated capacity of `load_status`.                       |
| topo           | [`*sess.ModuleId`](session.md#moduleid)           | ModuleIds in dependency order; `topo[0]` has no deps within the graph. |
| topo_len       | `u32`                                             | Number of entries in `topo`.                               |
| topo_cap       | `u32`                                             | Allocated capacity of `topo`.                              |

## Constants

```mach
val INITIAL_MODULE_CAP: u32 = 16;
val INITIAL_PUB_CAP:    u32 = 4;
val INITIAL_DEP_CAP:    u32 = 4;
```

Starting capacities for [`Project.modules`](#project) /
[`Project.load_status`](#project) / [`Project.topo`](#project), for
each module's [`pub_consts`](#moduleentry), and for each module's
[`deps`](#moduleentry) respectively. Each doubles on overflow.

```mach
def LoadStatus: u8;
val LOAD_NEW:     LoadStatus = 0;
val LOAD_LOADING: LoadStatus = 1;
val LOAD_DONE:    LoadStatus = 2;
```

DFS tristate stored in [`Project.load_status[mid]`](#project). See
[DFS load](#dfs-load) for the state machine.

## Functions

### `build_project`

```mach
pub fun build_project(
    s:            *sess.Session,
    project_root: str,
    target_name:  str,
) Result[Project, str]
```

Loads `project_root/mach.toml`, selects `target_name`, and builds the
full module graph plus per-module [`ResolveResult`](fe/resolve.md#resolveresult)s.
Runs [`init_project`](#internal-helpers) →
[`load_project_config`](#internal-helpers) →
[`find_target_entry`](#internal-helpers) →
[`select_target`](#target-selection) →
[`entry_module_fqn`](#fqn-and-file-paths) →
[`dfs_load`](#dfs-load) → [`run_resolve_pass`](#resolve-pass). On any
error the partial project is torn down via
[`dnit_project`](#dnit_project) and the error string is propagated.

| Param        | Type                                  | Description                                              |
|--------------|---------------------------------------|----------------------------------------------------------|
| s            | [`*sess.Session`](session.md#session) | Shared session that owns the build's sources, diags, interner, and type interner. |
| project_root | `str`                                 | Filesystem path to the project root (the directory containing `mach.toml`). |
| target_name  | `str`                                 | Selector that must match a `[targets.<name>]` entry in `mach.toml`. |

Returns the populated [`Project`](#project), or an error string. Errors
originate from filesystem I/O, TOML parsing, missing `mach.toml` keys,
unknown target name, unknown OS/ISA, module cycle, comptime evaluation
failure on a `$if` cond or pub comptime val, or any allocation failure.

### `dnit_project`

```mach
pub fun dnit_project(p: *Project)
```

Releases every resource owned by the project. Walks
[`p.modules`](#project) and dnit's each module's
[`resolve_result`](#moduleentry) (when `resolved`),
[`ctx`](#moduleentry), and [`a`](#moduleentry); then frees the
modules array, load-status array, topo array,
[`by_fqn`](#project) map, and the config's target/dep arrays.
Safe to call on a partially-constructed project (used by
[`build_project`](#build_project) on error paths). `nil` is a no-op.

| Param | Type                       | Description                          |
|-------|----------------------------|--------------------------------------|
| p     | [`*Project`](#project)     | Project to tear down. `nil` is a no-op. |

## Pipeline

[`build_project`](#build_project) runs three serial phases:

1. **Config.** [`load_project_config`](#internal-helpers) reads
   `project_root/mach.toml`, fills [`ProjectConfig`](#projectconfig)'s
   `[project]` fields, and recursively reads each
   `[deps.<alias>]`'s own `mach.toml` (via
   [`resolve_dep`](#internal-helpers)) to discover the dep's
   `[project].id` and `dir_src`. Then
   [`find_target_entry`](#internal-helpers) selects the matching
   [`TargetEntry`](#targetentry) and [`select_target`](#target-selection)
   resolves it into a [`tgt.Target`](target.md#target) stored on
   [`Project.target`](#project).
2. **DFS load.** [`dfs_load`](#dfs-load) starts at the target's
   entrypoint module (FQN computed by
   [`entry_module_fqn`](#fqn-and-file-paths)) and walks the module
   graph. Each module is parsed once; its decls are walked in source
   order; each active `use` recurses; `$if` predicates are evaluated
   against the running [`ComptimeCtx`](fe/comptime.md#comptimectx);
   cycles are reported. DFS post-order is the topological order, so
   `topo` is appended-to on the way back up.
3. **Resolve.** [`run_resolve_pass`](#resolve-pass) iterates
   [`Project.topo`](#project) and calls
   [`resolve.resolve`](fe/resolve.md#resolve) on each module, passing
   that module's already-resolved deps as
   [`ResolveDeps`](fe/resolve.md#resolvedeps).

## Target selection

[`select_target`](#internal-helpers) reads the active
[`TargetEntry`](#targetentry)'s `os_name` and `isa_name` strings out
of the interner, maps them to numeric ids via
[`os_id_for`](#internal-helpers) / [`arch_id_for`](#internal-helpers),
and hands them to
[`target.select`](target.md#select). The resulting
[`tgt.Target`](target.md#target) is stored on
[`Project.target`](#project) and consumed by every downstream phase
through the vtable surface.

| Name string | OS id                                            |
|-------------|--------------------------------------------------|
| `"linux"`   | [`os.OS_LINUX`](target/os.md#constants)          |
| `"darwin"`  | [`os.OS_DARWIN`](target/os.md#constants)         |
| `"windows"` | [`os.OS_WINDOWS`](target/os.md#constants)        |
| (other)     | [`os.OS_UNKNOWN`](target/os.md#constants)        |

| Name string | Arch id                                          |
|-------------|--------------------------------------------------|
| `"x86_64"`  | [`isa.ARCH_X86_64`](target/isa.md#constants)     |
| `"aarch64"` | [`isa.ARCH_AARCH64`](target/isa.md#constants)    |
| (other)     | [`isa.ARCH_UNKNOWN`](target/isa.md#constants)    |

An unknown arch fails the build via
[`target.select`](target.md#select) (no canonical ABI / OF mapping).
An unknown os is silently accepted as
[`OS_UNKNOWN`](target/os.md#constants); the comptime expression
`$mach.build.target.os == "linux"` then evaluates to `false`.

`compiler_name` / `compiler_ver` are filled with the interned strings
`"mach"` / `"0.0.0"`. [`comptime.init`](fe/comptime.md#init) on every
per-module context takes `*Target` plus the compiler-identity strings;
pointer width and endianness are read through
[`target.arch`](target/isa.md#isavtable) — they are not duplicated on
[`Project`](#project) or [`ComptimeCtx`](fe/comptime.md#comptimectx).

## DFS load

[`dfs_load`](#internal-helpers) is the only routine that creates
[`ModuleEntry`](#moduleentry) slots. It works in this state:

- `LOAD_NEW` — slot exists in [`by_fqn`](#project) but DFS hasn't
  reached this module yet, or the slot has not been allocated.
- `LOAD_LOADING` — DFS has registered the slot and is currently
  walking its decls. A second visit while in this state is the cycle
  case.
- `LOAD_DONE` — module has been parsed, walked, and pushed onto
  [`topo`](#project).

Per call, [`dfs_load`](#dfs-load) takes the module's
[`intern.StrId`](intern.md#strid) FQN and:

1. Looks the FQN up in [`by_fqn`](#project). On hit:
   - `LOAD_LOADING` → emit `"circular module dependency"`.
   - `LOAD_DONE` → return the existing ModuleId.
2. On miss, [`register_module_slot`](#internal-helpers) appends a
   fresh [`ModuleEntry`](#moduleentry) (its `ctx` initialized with the
   target params), records it in `by_fqn`, and returns the new
   ModuleId.
3. [`ensure_load_status_capacity`](#internal-helpers) grows
   `load_status` to fit the new id; the slot is set to `LOAD_LOADING`.
4. [`parse_module`](#internal-helpers) resolves the module's FQN to a
   file path via [`fqn_to_file_path`](#fqn-and-file-paths), reads the
   file, registers it with [`session.load_source`](session.md#load_source),
   tokenizes, and runs [`parser.parse`](fe/parser.md#parse) into
   `m.a`.
5. [`walk_module_for_load`](#internal-helpers) walks the module's root
   decl list (see [Decl walk](#decl-walk)).
6. The slot flips to `LOAD_DONE` and [`topo_push`](#internal-helpers)
   appends the ModuleId to [`Project.topo`](#project).

Modules already in [`by_fqn`](#project) (e.g. a dep loaded earlier)
short-circuit at step 1 with no re-walk.

## Decl walk

[`walk_one_decl_for_load`](#internal-helpers) special-cases three
[`DeclKind`](fe/ast/decl.md#declkind)s during DFS load; every other
kind is skipped at this phase (resolve handles them later):

| `DECL_KIND_*` | Action                                                                                |
|---------------|---------------------------------------------------------------------------------------|
| `COMPTIME_IF` | [`walk_comptime_if_for_load`](#internal-helpers): evaluate each arm's cond via [`comptime_branch_active_load`](#internal-helpers); recurse into the *first* matching arm's body. Subsequent arms are skipped. |
| `USE`         | [`walk_use_for_load`](#internal-helpers): intern the dotted path span, [`dfs_load`](#dfs-load) the target, record the dep via [`record_dep`](#internal-helpers), and [`merge_pub_consts`](#internal-helpers) the target's exported comptime constants into this module's [`ctx`](#moduleentry). |
| `VAL`         | [`register_val_for_load`](#internal-helpers): intern the name, [`comptime.eval`](fe/comptime.md#eval) the initializer against `m.ctx`. On success, [`comptime.register`](fe/comptime.md#register) it into `ctx`; if the decl is pub, also [`add_pub_const`](#internal-helpers) into `m.pub_consts` so importers can pick it up. On eval failure, emit a diagnostic at the initializer span and continue (non-comptime vals are normal and produce a non-fatal diagnostic that downstream phases ignore). |

`fwd <identifier>;` decls are intentionally not walked here. The
forwarded symbol's source module is reachable only because some
sibling `use` in the same module brought it into scope (and the dep
set) by the standard scope-driven path; resolve handles the actual
re-export. See [`resolve` — Re-export](fe/resolve.md#re-export-fwd).

[`comptime_branch_active_load`](#internal-helpers) treats an absent
cond ([`EXPR_NIL`](fe/ast/id.md#constants), the `or` arm of an `$if`)
as unconditionally true. An eval error or a non-bool result emits a
[`SEVERITY_ERROR`](diagnostic.md#constants) diagnostic at the cond's
span and treats the arm as inactive (so DFS continues to the next
arm rather than halting the build).

`use` decls with an empty path span are silently skipped — that case
covers malformed parses that have already emitted parser
diagnostics.

[`record_dep`](#internal-helpers) dedupes — the same dep used twice
in one module produces a single [`deps`](#moduleentry) entry. The
list is the basis for [`build_resolve_deps`](#internal-helpers).

## Comptime context per module

Each module owns its own [`ComptimeCtx`](fe/comptime.md#comptimectx)
on [`ModuleEntry.ctx`](#moduleentry). The driver populates it in this
order:

1. **Target params.** [`comptime.init`](fe/comptime.md#init) is
   called by [`register_module_slot`](#internal-helpers) with
   `*Project.target` plus `compiler_name` / `compiler_ver` — those
   become `$mach.build.target.*` and `$mach.compiler.*` lookups
   inside the module. Comptime reads pointer width / endianness /
   abi name / object format / etc. through the target's vtables.
2. **Imported pub consts.** As each `use` is walked,
   [`merge_pub_consts`](#internal-helpers) copies every
   [`PubConst`](#pubconst) entry from the dep's
   [`ModuleEntry.pub_consts`](#moduleentry) into this module's `ctx`
   via [`comptime.register`](fe/comptime.md#register). Because the dep
   was DFS-loaded first, its own pub consts have already been
   registered into its own ctx, copied into [`pub_consts`](#moduleentry)
   for export, and are available the moment its `use` site returns.
3. **Own pub vals.** [`register_val_for_load`](#internal-helpers)
   registers each successfully-evaluated pub `val` into this module's
   own `ctx` so a later `$if` in the same module can reference it.

Comptime values registered this way are what [resolve](fe/resolve.md)
later uses to drive its own `$if` evaluation inside function bodies —
the ctx passed to [`resolve.resolve`](fe/resolve.md#resolve) is the
same object the driver built up here.

## FQN and file paths

[`entry_module_fqn`](#internal-helpers) builds the entrypoint FQN as
`<config.id>.<entrypoint with .mach stripped and / or \ replaced by .>`
and interns it. For example, project id `"mach"` with entrypoint
`"cli/main.mach"` produces `"mach.cli.main"`.

[`fqn_to_file_path`](#internal-helpers) inverts this for an arbitrary
loaded module. It splits the FQN at the first `.`:

- If the head segment equals [`config.id`](#projectconfig), the path
  is `<project_root>/<config.dir_src>/<tail with . → />.mach`.
- Otherwise, the head is matched against each
  [`DepEntry.project_id`](#depentry); the path is
  `<dep.vendor_root>/<dep.project_dir_src>/<tail with . → />.mach`.
- No match emits `"no project or dep matches the head segment of
  the module path"`.

The composed path is allocated by [`build_module_path`](#internal-helpers)
and freed by [`parse_module`](#internal-helpers) once
[`session.load_source`](session.md#load_source) has copied the contents.

## Resolve pass

[`run_resolve_pass`](#internal-helpers) iterates
[`Project.topo`](#project) in DFS post-order and calls
[`run_resolve_one`](#internal-helpers) for each ModuleId.
[`run_resolve_one`](#internal-helpers):

1. Calls [`build_resolve_deps`](#internal-helpers) to materialize the
   module's [`ResolveDeps`](fe/resolve.md#resolvedeps). For every
   recorded dep, [`build_module_exports`](#internal-helpers) reads the
   dep's [`ResolveResult.symbols`](fe/resolve.md#resolveresult) (the
   first [`pub_count`](fe/resolve.md#resolveresult) entries are the
   pub-flagged symbols, in original declaration order), copies them
   into a fresh [`*PublicSymbol`](fe/resolve.md#publicsymbol) array,
   and stores them in a [`ModuleExports`](fe/resolve.md#moduleexports)
   record keyed by the dep's FQN.
2. Calls [`resolve.resolve`](fe/resolve.md#resolve) with the session,
   this module's `Ast`, the freshly-built `ResolveDeps`, this
   module's `ctx`, and its ModuleId.
3. [`free_resolve_deps`](#internal-helpers) releases the temporary
   `ResolveDeps` arrays.
4. On success, the [`ResolveResult`](fe/resolve.md#resolveresult) is
   stored on the entry and `resolved` flips true.

Because topo order is bottom-up, every dep is already
`resolved == true` by the time its dependent reaches step 1. A
non-resolved dep (which only happens if a prior topo iteration
failed) produces an empty [`ModuleExports`](fe/resolve.md#moduleexports)
entry; the resolve call will then emit `"imported module is not in
the dep set"`-style diagnostics for any references.

## Internal helpers

File-private; listed for reference.

| Function                          | Role                                                                          |
|-----------------------------------|-------------------------------------------------------------------------------|
| `init_project`                    | Zeroes a [`Project`](#project) and initializes [`by_fqn`](#project).          |
| `deallocate_config`               | Frees the config's targets/deps arrays and resets the counts.                 |
| `load_project_config`             | Reads `project_root/mach.toml`, fills the `[project]` fields, then calls `parse_targets_table` and `parse_deps_table`. |
| `parse_targets_table`             | Allocates [`config.targets`](#projectconfig) and walks `[targets.<name>]` entries. |
| `parse_deps_table`                | Allocates [`config.deps`](#projectconfig) and walks `[deps.<alias>]` entries, calling `resolve_dep` for each. |
| `resolve_dep`                     | Reads a dep's own `mach.toml` to recover its [`project.id`](#depentry) and [`project.dir_src`](#depentry); interns the vendor root. |
| `build_dep_root`                  | Composes `<project_root>/<dir_dep>/<alias>`.                                  |
| `parse_toml_file`                 | Reads a file via `fs.read_all` and parses with `toml.parse`.                  |
| `intern_or`                       | Interns `text`, or `default` if `text` is empty.                              |
| `find_target_entry`               | Linear scan of [`config.targets`](#projectconfig) for a name match.           |
| `select_target`                   | Resolves the matched [`TargetEntry`](#targetentry) into a [`tgt.Target`](target.md#target) via [`target.select`](target.md#select); also fills `compiler_name` / `compiler_ver`. See [Target selection](#target-selection). |
| `os_id_for`                       | Maps an OS name string to one of [`os.OS_*`](target/os.md#constants).         |
| `arch_id_for`                     | Maps an ISA name string to one of [`isa.ARCH_*`](target/isa.md#constants).    |
| `entry_module_fqn`                | Composes the entrypoint FQN from [`config.id`](#projectconfig) + [`target.entrypoint`](#targetentry). |
| `strip_mach_suffix`               | Returns `s` with a trailing `".mach"` removed (or `s` unchanged).             |
| `fqn_to_file_path`                | Resolves a module FQN to an allocated absolute file path. See [FQN and file paths](#fqn-and-file-paths). |
| `build_module_path`               | Composes `<root>/<src_dir>/<dotted tail with / separators>.mach`.             |
| `join_path`                       | Joins two segments with a single `/`.                                         |
| `ensure_load_status_capacity`     | Grows [`load_status`](#project) to fit `needed` slots; new slots start at `LOAD_NEW`. |
| `dfs_load`                        | DFS-loads one module by FQN. See [DFS load](#dfs-load).                       |
| `register_module_slot`            | Appends a fresh [`ModuleEntry`](#moduleentry) and inserts it in [`by_fqn`](#project). Also calls [`comptime.init`](fe/comptime.md#init) on `m.ctx` with the target params. |
| `parse_module`                    | Reads the file, registers it with [`session.load_source`](session.md#load_source), lexes via [`lexer.tokenize`](fe/lexer.md), and runs [`parser.parse`](fe/parser.md#parse) into `m.a`. |
| `walk_module_for_load`            | Entry point into the decl walk; resolves the root [`Module`](fe/ast/module.md) node and forwards to `walk_decl_list_for_load`. |
| `walk_decl_list_for_load`         | Loops `walk_one_decl_for_load` over `[start, start+len)` of [`Ast.decl_ids`](fe/ast.md#ast). |
| `walk_one_decl_for_load`          | Dispatch per [`DeclKind`](fe/ast/decl.md#declkind) during load. See [Decl walk](#decl-walk). |
| `walk_comptime_if_for_load`       | Walks `$if` arms, evaluates each cond, recurses into the first active arm.    |
| `comptime_branch_active_load`     | Evaluates one arm's cond via [`comptime.eval`](fe/comptime.md#eval); errors and non-bool results emit a diagnostic and return `ok(false)`. |
| `walk_use_for_load`               | Resolves a `use` decl's target, DFS-loads it, and merges its pub consts into this module's ctx. |
| `record_dep`                      | Appends a ModuleId to [`m.deps`](#moduleentry) (dedup'd).                     |
| `merge_pub_consts`                | Calls [`comptime.register`](fe/comptime.md#register) for every [`PubConst`](#pubconst) in a dep's exports. |
| `register_val_for_load`           | Per-`val`-decl handler: evaluates the initializer, registers it in this module's ctx, and (if pub) exports it via `add_pub_const`. |
| `add_pub_const`                   | Appends a [`PubConst`](#pubconst) to [`m.pub_consts`](#moduleentry); grows the array as needed. |
| `topo_push`                       | Appends a ModuleId to [`Project.topo`](#project); grows the array as needed.  |
| `run_resolve_pass`                | Iterates [`Project.topo`](#project) and calls `run_resolve_one` for each.     |
| `run_resolve_one`                 | Builds [`ResolveDeps`](fe/resolve.md#resolvedeps), calls [`resolve.resolve`](fe/resolve.md#resolve), stores the result on the entry. |
| `build_resolve_deps`              | Allocates a [`*ModuleExports`](fe/resolve.md#moduleexports) entry per recorded dep via `build_module_exports`. |
| `build_module_exports`            | Snapshots a resolved module's first [`pub_count`](fe/resolve.md#resolveresult) symbols into a fresh [`*PublicSymbol`](fe/resolve.md#publicsymbol) array. |
| `free_resolve_deps`               | Releases the per-call [`ResolveDeps`](fe/resolve.md#resolvedeps) arrays.      |

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.zstr`, `std.types.option`, `std.types.result`,
`std.allocator`, `std.filesystem`, `std.data.toml`,
`std.collections.map`,
[`mach.lang.source`](source.md), [`mach.lang.diagnostic`](diagnostic.md),
[`mach.lang.intern`](intern.md), [`mach.lang.session`](session.md),
[`mach.lang.fe.lexer`](fe/lexer.md), [`mach.lang.fe.parser`](fe/parser.md),
[`mach.lang.fe.ast`](fe/ast.md), [`mach.lang.fe.ast.id`](fe/ast/id.md),
[`mach.lang.fe.ast.module`](fe/ast/module.md),
[`mach.lang.fe.ast.decl`](fe/ast/decl.md),
[`mach.lang.fe.ast.stmt`](fe/ast/stmt.md),
[`mach.lang.fe.ast.expr`](fe/ast/expr.md),
[`mach.lang.fe.ast.type`](fe/ast/type.md),
[`mach.lang.fe.token`](fe/token.md),
[`mach.lang.fe.comptime`](fe/comptime.md),
[`mach.lang.fe.resolve`](fe/resolve.md),
[`mach.lang.target`](target.md),
[`mach.lang.target.os`](target/os.md),
[`mach.lang.target.isa`](target/isa.md).
