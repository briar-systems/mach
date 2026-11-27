[mach](../README.md) / [package](README.md) / **resolver**

# mach.package.resolver

dependency version resolution: PubGrub over the releases each identity lists

the algorithm is Weizenbaum's (2018), as in Dart's pub, over finite domains. an
identity's domain is its listed releases, so a set of versions is a bitset over them and
complement is exact. the running compiler is checked per release as its own
incompatibility, and a failure is explained from the derivation that proved it.

Source: [`src/package/resolver.mach`](../../../src/package/resolver.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.manifest.version`](../manifest/version.md), [`mach.package.source`](source.md).

| Type | | |
| :--- | :--- | :--- |
| [`Need`](#need) | `rec` | `who` names the requirer a diagnostic attributes a root need to: the root itself ("" reads as the problem's root) or the chain of a dependency that declared the range, since every requirer's range takes part in the root's problem |
| [`ReleaseNeeds`](#releaseneeds) | `rec` | what a release asks for: the compiler range its manifest states ("" for none, and `mach_range` it parsed, none for none) and its dependencies selected by version range |
| [`Lock`](#lock) | `rec` | a preferred version, kept while it still satisfies every requirement (`update <name>`) |
| [`Problem`](#problem) | `rec` | fixed ids are identities the root, or a dependency it reaches by ref or path, selects by ref or path: that selection overrides them, so no requirement on them takes part in resolution |
| [`Choice`](#choice) | `rec` | the release resolution selected for one identity, its texts owned by the caller's allocator |

| On `Problem` | |
| :--- | :--- |
| [`resolve`](#resolve) | the selected release of every identity the root reaches, allocated in `a`; the solver's scratch arena grows from `a` and returns to it |

---

## Need

```mach
pub rec Need {
    id:     str;
    url:    str;
    range:  str;
    admits: mach.manifest.version.Range;
    who:    str;
}
```

`who` names the requirer a diagnostic attributes a root need to: the root itself
("" reads as the problem's root) or the chain of a dependency that declared the
range, since every requirer's range takes part in the root's problem. `range` is the range
as written and `admits` it parsed where it entered, borrowing from it

## ReleaseNeeds

```mach
pub rec ReleaseNeeds {
    mach:       str;
    mach_range: opt[mach.manifest.version.Range];
    needs:      Vector[Need];
    excluded:   str;
    detail:     str;
}
```

what a release asks for: the compiler range its manifest states ("" for none, and
`mach_range` it parsed, none for none) and its
dependencies selected by version range. `excluded` is "" for a candidate and otherwise says
why the release is not one (its manifest does not load, or names another version): the
release is set aside and the reason explains a failure it takes part in, while a failure to
read the release at all is the NeedsFn's err and stops resolution. `detail` is what the
manifest's load said, or nil

## Lock

```mach
pub rec Lock {
    id:      str;
    version: str;
}
```

a preferred version, kept while it still satisfies every requirement (`update <name>`)

## Problem

```mach
pub rec Problem {
    root:        str;
    needs:       *Need;
    need_count:  u32;
    fixed:       *str;
    fixed_count: u32;
    compiler:    str;
    lowest:      bool;
    locks:       *Lock;
    lock_count:  u32;
}
```

fixed ids are identities the root, or a dependency it reaches by ref or path, selects by ref
or path: that selection overrides them, so no requirement on them takes part in resolution

### resolve

```mach
pub fun resolve(a: *std.allocator.Allocator, src: *mach.package.source.Releases, needs: NeedsFn, needs_ctx: ptr,
    problem: *Problem) res[Vector[Choice], mach.base.fail.Fail]
```

the selected release of every identity the root reaches, allocated in `a`;
the solver's scratch arena grows from `a` and returns to it

## Choice

```mach
pub rec Choice {
    id:      str;
    url:     str;
    tag:     str;
    commit:  str;
    version: str;
    chain:   str;
}
```

the release resolution selected for one identity, its texts owned by the caller's allocator

| Field | Type | |
| :--- | :--- | :--- |
| `id` | `str` | the identity |
| `url` | `str` | the source it is listed at |
| `tag` | `str` | the tag that names the release |
| `commit` | `str` | the commit the tag points at |
| `version` | `str` | the release's version text |
| `chain` | `str` | the projects from the root that require it, the root first and the identity last |

