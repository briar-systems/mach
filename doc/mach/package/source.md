[mach](../README.md) / [package](README.md) / **source**

# mach.package.source

the source contract: where a dependency named by a url comes from. it has two halves,
each a table of operations a source member fills:

- Releases: the releases published at a url and the manifest each ships, which version
  resolution reads
- Checkouts: how dep/\<id> holds a source's content: bringing it to a selection, saying what
  it holds, and checking it against what selected it

git is the member today: its releases are `v`-prefixed semver tags and its checkouts are
submodules. a source served over http would list releases from an index and unpack a
release archive into the same slot, filling the same two tables. a selector beyond a
release (a `ref` naming a branch, a tag or a commit) is a source's own vocabulary, and a
member without one refuses it

Source: [`src/package/source.mach`](../../../src/package/source.mach)

Uses types from [`mach.manifest.model`](../manifest/model.md), [`mach.manifest.version`](../manifest/version.md).

| Type | | |
| :--- | :--- | :--- |
| [`Release`](#release) | `rec` | one release: its version (whose pre-release text borrows from `text`), the tag that names it and the commit it points at, all owned by the list's allocator |
| [`Shipped`](#shipped) | `rec` | a release's manifest as resolution reads it: `manifest` when the release is a candidate, and otherwise `excluded` says why it is not one, its manifest not loading or naming another version |
| [`Releases`](#releases) | `rec` |  |
| [`Acquisition`](#acquisition) | `rec` | one acquisition of dep/\<id>: bring it to `selector` from `url` and record its pin as the project records pins (`mode`, a pin.mach mode) |
| [`Checkouts`](#checkouts) | `rec` |  |

| On `Release` | |
| :--- | :--- |
| [`releases_dnit`](#releases_dnit) | |

| Constants | |
| :--- | :--- |
| [`REALIZED_NOTHING`](#constants) | what acquiring dep/\<id> changed |
| [`REALIZED_INITIALIZED`](#constants), [`REALIZED_PINNED`](#constants), [`REALIZED_REGISTERED`](#constants), [`REALIZED_ADOPTED`](#constants), [`REALIZED_CLONED`](#constants) | |
| [`REALIZED_UNPINNED`](#constants) | nothing pins the dependency and nothing holds it, so a selection by range alone has nothing to realize |

---

## Release

```mach
pub rec Release {
    version: mach.manifest.version.Version;
    text:    str;
    tag:     str;
    commit:  str;
}
```

one release: its version (whose pre-release text borrows from `text`), the tag that
names it and the commit it points at, all owned by the list's allocator

### releases_dnit

```mach
pub fun releases_dnit(a: *std.allocator.Allocator, list: *Vector[Release])
```

## Shipped

```mach
pub rec Shipped {
    manifest: mach.manifest.model.Manifest;
    excluded: str;
}
```

a release's manifest as resolution reads it: `manifest` when the release is a candidate,
and otherwise `excluded` says why it is not one, its manifest not loading or naming
another version. the manifest is owned by the caller

## Releases

```mach
pub rec Releases {
    ctx:        ptr;
    list:       ListFn;
    shipped:    ShippedFn;
    release_of: ReleaseOfFn;
    remember:   RememberFn;
    close:      ReleasesCloseFn;
}
```

## Acquisition

```mach
pub rec Acquisition {
    root:        str;
    id:          str;
    url:         str;
    selector:    str;
    mode:        u8;
    accept:      bool;
    offline:     bool;
    pinned_only: bool;
}
```

one acquisition of dep/\<id>: bring it to `selector` from `url` and record its pin as the
project records pins (`mode`, a pin.mach mode). `accept` takes an existing checkout as the
dependency, as `mach dep add` does; `pinned_only` is a selection by range alone, which
acquires only what a pin or an existing checkout already names

## Checkouts

```mach
pub rec Checkouts {
    ctx:          ptr;
    acquire:      AcquireFn;
    revision:     RevisionFn;
    advance:      AdvanceFn;
    hydrate:      HydrateFn;
    selection:    SelectionFn;
    manifest_of:  ManifestOfFn;
    release_at:   ReleaseAtFn;
    at_release:   AtReleaseFn;
    has_releases: HasReleasesFn;
    holds:        HoldsFn;
    verify:       VerifyFn;
    satisfies:    SatisfiesFn;
    close:        CloseFn;
}
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `REALIZED_NOTHING` | `u8` | `0` | what acquiring dep/\<id> changed |
| `REALIZED_INITIALIZED` | `u8` | `1` |  |
| `REALIZED_PINNED` | `u8` | `2` |  |
| `REALIZED_REGISTERED` | `u8` | `3` |  |
| `REALIZED_ADOPTED` | `u8` | `4` |  |
| `REALIZED_CLONED` | `u8` | `5` |  |
| `REALIZED_UNPINNED` | `u8` | `6` | nothing pins the dependency and nothing holds it, so a selection by range alone has nothing to realize |

