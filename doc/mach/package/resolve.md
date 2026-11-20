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
| [`OutdatedRow`](#outdatedrow) | `rec` | one version-selected identity as `mach dep outdated` shows it: the release its pin is ("-" for none), the highest this compiler and every range accept, and the highest published |
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
pub fun versions(op: *mach.package.closure.Operation, doc: *mach.manifest.load.Doc, lowest: bool, lock: bool, unlocked: str,
    root_notes: bool, walk: *mach.package.closure.Walk) res[Resolution, mach.base.fail.Fail]
```

resolve every identity the closure reaches through a version range: the root's
own ranges and every range a dependency reached by a ref or a path declares,
read from the same closure walk `pull` realizes. an identity the root or such a
dependency selects by ref or path is fixed, and a requirement on it is
overridden. locks keep each realized version-selected checkout where it is
unless `unlocked` names it or `lock` is false; an identity with no checkout yet
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
for none), the highest this compiler and every range accept, and the highest published

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

