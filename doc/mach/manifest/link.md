[mach](../README.md) / [manifest](README.md) / **link**

# mach.manifest.link

the `[link.<name>]` tables: their decoding, and their resolution for one target

an entry applies to a target its `sys`, `arch` and `abi` filters all admit, and a
`LinkRequirement` owns its copy of the entry's symbols.

Source: [`src/manifest/link.mach`](../../../src/manifest/link.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.manifest.template`](template.md), [`mach.target.input`](../target/input.md).

| Type | | |
| :--- | :--- | :--- |
| [`LinkRequirement`](#linkrequirement) | `rec` | a `[link.<name>]` entry resolved for one target, as the linker consumes it |

| On `LinkRequirement` | |
| :--- | :--- |
| [`link_claims_dnit`](#link_claims_dnit) | free the symbol arrays of `count` requirements and nil them |
| [`finish_link_requirements`](#finish_link_requirements) | turn a partially filled requirement buffer into an exactly sized result, taking ownership of the buffer |
| [`merge_link_claims`](#merge_link_claims) | add every symbol of `src` that `dst` lacks to `dst`, keeping `dst` order first and the new entries in `src` order |
| [`link_requirement`](#link_requirement) | resolve a link entry for one target into a `LinkRequirement` |

| Functions | |
| :--- | :--- |
| [`parse_links`](#parse_links) | decode every `[link.<name>]` table of `t` into `m.links`, nothing when there is none |
| [`link_matches_target`](#link_matches_target) | whether a link's `sys`, `arch` and `abi` filters all admit a target |
| [`expand_local_path`](#expand_local_path) | expand a local link's `path` with `v`; a refusal points at the `path` value |
| [`find_link_by_name`](#find_link_by_name) | the link `m` declares under the interned `name`, nil when none is |

---

## LinkRequirement

```mach
pub rec LinkRequirement {
    source: LinkSource;
    text:   mach.base.intern.StrId;
    dir:    mach.base.intern.StrId;
    key:    mach.base.intern.StrId;
    at:     mach.base.source.Location;

    symbols:            *mach.base.intern.StrId;
    symbol_count:       u32;
    include_referenced: bool;
    kind:               mach.target.input.Kind;
    versions:           LinkVersions;
}
```

a `[link.<name>]` entry resolved for one target, as the linker consumes it.
the symbol array is owned; release it with `link_claims_dnit`

| Field | Type | |
| :--- | :--- | :--- |
| `source` | `LinkSource` | copied from the link |
| `text` | `mach.base.intern.StrId` | for a local source, the `path` expanded, anchored as an absolute path and interned; otherwise the link's `name` |
| `dir` | `mach.base.intern.StrId` | for a framework source stating `dir`, that directory expanded, anchored and interned as `text` is for a local source; STR_NIL otherwise |
| `key` | `mach.base.intern.StrId` | the link's table key, its identity |
| `at` | `mach.base.source.Location` | where the link is stated, in the manifest that states it |
| `symbols` | `*mach.base.intern.StrId` | an owned copy of the link's `symbols`, or nil when it has none |
| `symbol_count` | `u32` | length of `symbols` |
| `include_referenced` | `bool` | copied from the link |
| `kind` | `mach.target.input.Kind` | copied from the link |
| `versions` | `LinkVersions` | copied from the link |

### link_claims_dnit

```mach
pub fun link_claims_dnit(alloc: *std.allocator.Allocator, items: *LinkRequirement, count: u32)
```

free the symbol arrays of `count` requirements and nil them. the `items` array
itself is not freed

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator the claims were made from |
| `items` | the requirements; nil is accepted |
| `count` | how many entries to release |

### finish_link_requirements

```mach
pub fun finish_link_requirements(alloc: *std.allocator.Allocator, items: *LinkRequirement,
    capacity: u32, count: u32,
    out_items: **LinkRequirement,
    out_count: *u32) err[mach.base.fail.Fail]
```

turn a partially filled requirement buffer into an exactly sized result, taking
ownership of the buffer

| Parameter | |
| :--- | :--- |
| `alloc` | the buffer's allocator |
| `items` | a buffer of `capacity` entries with the first `count` filled |
| `capacity` | the buffer's allocated extent |
| `count` | the filled prefix |
| `out_items` | receives the exact array, or nil when `count` is 0 or on error |
| `out_count` | receives `count`, or 0 when it is 0 or on error |
| **returns** | ok; when `count` is 0 the buffer is freed. on a failed shrink the filled claims and the whole buffer are freed and the allocator's error is returned |

### merge_link_claims

```mach
pub fun merge_link_claims(alloc: *std.allocator.Allocator, dst: *LinkRequirement,
    src: *LinkRequirement) err[mach.base.fail.Fail]
```

add every symbol of `src` that `dst` lacks to `dst`, keeping `dst` order first
and the new entries in `src` order. `src` is not modified

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator of `dst.symbols`; the old array is freed and replaced |
| `dst` | the requirement that grows |
| `src` | the requirement whose symbols are added |
| **returns** | ok, without allocating, when nothing is new; err on a symbol count overflow or allocation failure, leaving `dst` unchanged |

### link_requirement

```mach
pub fun link_requirement(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, l: *LinkDef,
    proj_out: str, v: *mach.manifest.template.Values, declared: str, home: str) res[LinkRequirement, mach.base.fail.Fail]
```

resolve a link entry for one target into a `LinkRequirement`. a local `path`
and a framework's `dir` are expanded, anchored and interned, and the symbols are
copied into an owned array. the path is anchored once, here: at the root
project's directory when its template names a variable whose value lies under
it, as `{project.work}` does, and otherwise at the directory of the project
whose manifest states the entry

| Parameter | |
| :--- | :--- |
| `alloc` | owns the symbol copy |
| `itn` | interns the anchored path |
| `l` | the link |
| `proj_out` | the expanded `[project].work` |
| `v` | the template values |
| `declared` | the directory of the project whose manifest states `l` |
| `home` | the root project's directory |
| **returns** | the requirement; on error nothing is left allocated |

## Functions

### parse_links

```mach
pub fun parse_links(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

decode every `[link.<name>]` table of `t` into `m.links`, nothing when there is none

| Parameter | |
| :--- | :--- |
| `alloc` | owns the link array, its entries and a refusal's text |
| `itn` | interns the names, paths and filter values |
| `t` | the manifest's root table |
| `m` | the manifest the links are set on |
| **returns** | ok, or the refusal of the first link that does not decode |

### link_matches_target

```mach
pub fun link_matches_target(itn: *mach.base.intern.Interner, l: *LinkDef, t: *TargetDef) bool
```

whether a link's `sys`, `arch` and `abi` filters all admit a target. an empty
axis admits nothing, "*" everything

| Parameter | |
| :--- | :--- |
| `itn` | resolves the filter values |
| `l` | the link |
| `t` | the target |
| **returns** | true when all three axes admit `t` |

### expand_local_path

```mach
pub fun expand_local_path(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, l: *LinkDef,
    proj_out: str, v: *mach.manifest.template.Values) res[str, mach.base.fail.Fail]
```

expand a local link's `path` with `v`; a refusal points at the `path` value

### find_link_by_name

```mach
pub fun find_link_by_name(m: *Manifest, name: mach.base.intern.StrId) *LinkDef
```

the link `m` declares under the interned `name`, nil when none is

