# mach.lang.package.source

the source contract: where a dependency named by a url comes from. it has two halves,
each a table of operations a source member fills:

- Releases: the releases published at a url and the manifest each ships, which version
  resolution reads
- Checkouts: how dep/<id> holds a source's content: bringing it to a selection, saying what
  it holds, and checking it against what selected it

git is the member today: its releases are `v`-prefixed semver tags and its checkouts are
submodules. a source served over http would list releases from an index and unpack a
release archive into the same slot, filling the same two tables. a selector beyond a
release (a `ref` naming a branch, a tag or a commit) is a source's own vocabulary, and a
member without one refuses it

## rec Release

```mach
pub rec Release;
```

one release: its version (whose pre-release text borrows from `text`), the tag that
names it and the commit it points at, all owned by the list's allocator

## rec Shipped

```mach
pub rec Shipped;
```

a release's manifest as resolution reads it: `manifest` when the release is a candidate,
and otherwise `excluded` says why it is not one, its manifest not loading or naming
another version. the manifest is owned by the caller

## rec Releases

```mach
pub rec Releases;
```

## val REALIZED_NOTHING

```mach
pub val REALIZED_NOTHING:     u8 = 0
```

what acquiring dep/<id> changed

## val REALIZED_INITIALIZED

```mach
pub val REALIZED_INITIALIZED: u8 = 1
```

## val REALIZED_PINNED

```mach
pub val REALIZED_PINNED:      u8 = 2
```

## val REALIZED_REGISTERED

```mach
pub val REALIZED_REGISTERED:  u8 = 3
```

## val REALIZED_ADOPTED

```mach
pub val REALIZED_ADOPTED:     u8 = 4
```

## val REALIZED_CLONED

```mach
pub val REALIZED_CLONED:      u8 = 5
```

## val REALIZED_UNPINNED

```mach
pub val REALIZED_UNPINNED: u8 = 6
```

nothing pins the dependency and nothing holds it, so a selection by range alone has nothing to realize

## rec Acquisition

```mach
pub rec Acquisition;
```

one acquisition of dep/<id>: bring it to `selector` from `url` and record its pin as the
project records pins (`mode`, a pin.mach mode). `accept` takes an existing checkout as the
dependency, as `mach dep add` does; `pinned_only` is a selection by range alone, which
acquires only what a pin or an existing checkout already names

## rec Checkouts

```mach
pub rec Checkouts;
```

## fun releases_free

```mach
pub fun releases_free(a: *A.Allocator, list: *Vector[Release]);
```

