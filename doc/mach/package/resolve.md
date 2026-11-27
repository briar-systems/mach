[mach](../README.md) / [package](README.md) / **resolve**

# mach.package.resolve

resolve: version resolution over a project's closure. every identity the closure reaches
through a version range resolves together against the releases its source publishes
(resolver.mach); an identity the root or a fixed edge selects by ref or path is fixed, and a
requirement on it is overridden

Source: [`src/package/resolve.mach`](../../../src/package/resolve.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.manifest.load`](../manifest/load.md), [`mach.package.closure`](closure.md), [`mach.package.resolver`](resolver.md).

| Type | | |
| :--- | :--- | :--- |
| [`Resolution`](#resolution) | `rec` | notes are the lines a command prints unless --quiet: an --offline resolution, and each requirement a root `ref` or `path` overrides |
| [`OutdatedRow`](#outdatedrow) | `rec` | one version-selected identity as `mach dep outdated` shows it: the release its pin is ("-" for none, "unknown" when it cannot be read), the highest this compiler and every range accept, and the highest published |
| [`Outdated`](#outdated) | `rec` | what `mach dep outdated` shows: the resolution's notes, whether any identity selects releases by version, and a row for each that does |

| On `Resolution` | |
| :--- | :--- |
| [`versions`](#versions) | resolve every identity the closure reaches through a version range: the root's own ranges and every range a dependency reached by a ref or a path declares, read from the same closure walk `pull` realizes |

| On `Outdated` | |
| :--- | :--- |
| [`outdated`](#outdated-1) | each version-selected identity's pinned release, the highest this compiler and every range accept, and the highest release published |

| Functions | |
| :--- | :--- |
| [`pick_index`](#pick_index) | the position of an identity among a resolution's picks |
| [`held_locks`](#held_locks) | the release each realized version-selected checkout of the closure `doc` reaches is pinned at, but `unlocked`'s, read before a command moves anything so that it moves no dependency it was not asked to: a pin that cannot be read, or that no release tag names once its tags are fetched, refuses the command |
| [`held_dnit`](#held_dnit) | release the locks `held_locks` read |

---

## Resolution

```mach
pub rec Resolution {
    picks:  Vector[mach.package.resolver.Choice];
    ranged: bool;
    notes:  Vector[str];
}
```

notes are the lines a command prints unless --quiet: an --offline resolution, and each
requirement a root `ref` or `path` overrides

### versions

```mach
pub fun versions(op: *mach.package.closure.Operation, doc: *mach.manifest.load.Doc, lowest: bool, held: *Vector[mach.package.resolver.Lock],
    unlocked: str, root_notes: bool, walk: *mach.package.closure.Walk) res[Resolution, mach.base.fail.Fail]
```

resolve every identity the closure reaches through a version range: the root's
own ranges and every range a dependency reached by a ref or a path declares,
read from the same closure walk `pull` realizes. an identity the root or such a
dependency selects by ref or path is fixed, and a requirement on it is
overridden. `held`, read by `held_locks` before the command moved anything, keeps each
realized version-selected checkout where it is, and nil locks nothing; an identity with no checkout yet
is seeded by the pin of the dependency declaring it, when one exists and the range
admits it, so a dependency pulls its own tested pin in

`root_notes` names the root's own overrides of a chosen release's requirements too; a
command that pulls the closure afterwards leaves them to the pull, which names every
requirement the root overrides

## OutdatedRow

```mach
pub rec OutdatedRow {
    id:         str;
    pinned:     str;
    compatible: str;
    latest:     str;
}
```

one version-selected identity as `mach dep outdated` shows it: the release its pin is ("-"
for none, "unknown" when it cannot be read), the highest this compiler and every range accept, and the highest published

## Outdated

```mach
pub rec Outdated {
    notes:  Vector[str];
    ranged: bool;
    rows:   Vector[OutdatedRow];
}
```

what `mach dep outdated` shows: the resolution's notes, whether any identity selects
releases by version, and a row for each that does

### outdated

```mach
pub fun outdated(op: *mach.package.closure.Operation, doc: *mach.manifest.load.Doc, offline: bool) res[Outdated, mach.base.fail.Fail]
```

each version-selected identity's pinned release, the highest this compiler and every range
accept, and the highest release published

## Functions

### pick_index

```mach
pub fun pick_index(picks: *Vector[mach.package.resolver.Choice], id: str) i64
```

the position of an identity among a resolution's picks

| Parameter | |
| :--- | :--- |
| `picks` | the chosen releases |
| `id` | the identity to find |
| **returns** | its index, -1 when resolution picked no release for it |

### held_locks

```mach
pub fun held_locks(op: *mach.package.closure.Operation, doc: *mach.manifest.load.Doc, unlocked: str, offline: bool, out: *Vector[mach.package.resolver.Lock]) err[mach.base.fail.Fail]
```

the release each realized version-selected checkout of the closure `doc` reaches is
pinned at, but `unlocked`'s, read before a command moves anything so that it moves no
dependency it was not asked to: a pin that cannot be read, or that no release tag names
once its tags are fetched, refuses the command. every pin is read before any checkout
fetches tags, and `offline` fetches none

| Parameter | |
| :--- | :--- |
| `op` | the operation |
| `doc` | the manifest as it stands before the command |
| `unlocked` | the identity the command may move, nil for none |
| `offline` | fetch no tags |
| `out` | receives each lock, its id and version owned, freed by `held_dnit` |
| **returns** | ok, or the refusal naming the dependency whose pin cannot be read |

### held_dnit

```mach
pub fun held_dnit(op: *mach.package.closure.Operation, held: *Vector[mach.package.resolver.Lock])
```

release the locks `held_locks` read

