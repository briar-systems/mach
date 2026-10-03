# mach.lang.package.resolve

resolve: version resolution over a project's closure. every identity the closure reaches
through a version range resolves together against the releases its source publishes
(resolver.mach); an identity the root or a fixed edge selects by ref or path is fixed, and a
requirement on it is overridden

## rec Resolution

```mach
pub rec Resolution;
```

notes are the lines a command prints unless --quiet: an --offline resolution, and each
requirement a root `ref` or `path` overrides

## fun pick_index

```mach
pub fun pick_index(picks: *Vector[resolver.Choice], id: str) i64;
```

## fun versions

```mach
pub fun versions(op: *package_closure.Operation, doc: *manifest.Doc, lowest: bool, lock: bool, unlocked: str,
root_notes: bool, walk: *package_closure.Walk) res[Resolution, fail.Fail];
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

## rec OutdatedRow

```mach
pub rec OutdatedRow;
```

one version-selected identity as `mach dep outdated` shows it: the release its pin is ("-"
for none), the highest this compiler and every range accept, and the highest published

## rec Outdated

```mach
pub rec Outdated;
```

what `mach dep outdated` shows: the resolution's notes, whether any identity selects
releases by version, and a row for each that does

## fun outdated

```mach
pub fun outdated(op: *package_closure.Operation, doc: *manifest.Doc, offline: bool) res[Outdated, fail.Fail];
```

each version-selected identity's pinned release, the highest this compiler and every range
accept, and the highest release published

