# mach.lang.manifest.link

## rec LinkRequirement

```mach
pub rec LinkRequirement;
```

a `[link.<name>]` entry resolved for one target, as the linker consumes it.
the symbol array is owned; release it with `free_link_claims`

source: copied from the link
text: for a local source, the `path` expanded and interned; otherwise the link's `name`
library: the link's logical `library` name
symbols: an owned copy of the link's `symbols`, or nil when it has none
symbol_count: length of `symbols`
include_referenced: copied from the link

## fun parse_links

```mach
pub fun parse_links(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest, as_root: bool) err[outcome.Fail];
```

## fun free_link_claims

```mach
pub fun free_link_claims(alloc: *A.Allocator, items: *LinkRequirement, count: u32);
```

free the symbol arrays of `count` requirements and nil them. the `items` array
itself is not freed

alloc: the allocator the claims were made from
items: the requirements; nil is accepted
count: how many entries to release

## fun finish_link_requirements

```mach
pub fun finish_link_requirements(alloc: *A.Allocator, items: *LinkRequirement,
capacity: u32, count: u32,
out_items: **LinkRequirement,
out_count: *u32) err[outcome.Fail];
```

turn a partially filled requirement buffer into an exactly sized result, taking
ownership of the buffer

alloc: the buffer's allocator
items: a buffer of `capacity` entries with the first `count` filled
capacity: the buffer's allocated extent
count: the filled prefix
out_items: receives the exact array, or nil when `count` is 0 or on error
out_count: receives `count`, or 0 when it is 0 or on error
ret: ok; when `count` is 0 the buffer is freed. on a failed shrink the filled
           claims and the whole buffer are freed and the allocator's error is returned

## fun merge_link_claims

```mach
pub fun merge_link_claims(alloc: *A.Allocator, dst: *LinkRequirement,
src: *LinkRequirement) err[outcome.Fail];
```

add every symbol of `src` that `dst` lacks to `dst`, keeping `dst` order first
and the new entries in `src` order. `src` is not modified

alloc: the allocator of `dst.symbols`; the old array is freed and replaced
dst: the requirement that grows
src: the requirement whose symbols are added
ret: ok, without allocating, when nothing is new; err on a symbol count
       overflow or allocation failure, leaving `dst` unchanged

## fun link_matches_target

```mach
pub fun link_matches_target(itn: *intern.Interner, l: *LinkDef, t: *TargetDef) bool;
```

whether a link's `os`, `isa` and `abi` filters all admit a target. an absent
axis admits everything, an empty one nothing, "*" everything

itn: interns "*"
l: the link
t: the target
ret: true when all three axes admit `t`

## fun link_requirement

```mach
pub fun link_requirement(alloc: *A.Allocator, itn: *intern.Interner, l: *LinkDef,
proj_out: str, v: *TmplVars) res[LinkRequirement, outcome.Fail];
```

resolve a link entry for one target into a `LinkRequirement`. a local `path`
is expanded with `expand_project_path` and interned; the symbols are copied
into an owned array

alloc: owns the symbol copy
itn: interns the expanded path
l: the link
proj_out: the expanded `[project].out`
v: the template values
ret: the requirement; on error nothing is left allocated

## fun find_link_by_name

```mach
pub fun find_link_by_name(m: *Manifest, name: intern.StrId) *LinkDef;
```

