[mach](../../README.md) / [package](../README.md) / [git](README.md) / **checkout**

# mach.package.git.checkout

checkout: the git half of the source contract that realizes dep/\<id>. a dependency is
a submodule: its slot is observed (what dep/\<id> holds and what the enclosing repository
records for it), refused where realizing it would replace work mach did not make, and
brought to its pin or selector by initializing, checking out, adopting or adding it. a
selector is a `ref` (`branch/`, `tag/` or `commit/`), and a release is read from the
`v`-prefixed tags a checkout holds

Source: [`src/package/git/checkout.mach`](../../../../src/package/git/checkout.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.manifest.load`](../../manifest/load.md), [`mach.package.git`](../git.md), [`mach.package.report`](../report.md), [`mach.package.source`](../source.md), [`mach.session`](../../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Slot`](#slot) | `rec` | a git dependency's slot: what dep/\<id> holds, and its record in the enclosing repository |

| On `Slot` | |
| :--- | :--- |
| [`slot_dnit`](#slot_dnit) | free what an observed slot owns and leave it empty |
| [`observe_slot`](#observe_slot) | observe dep/\<id>: what the directory holds and, when the project records pins, its registration in .gitmodules, its module store and its committed gitlink |
| [`realize_slot`](#realize_slot) | bring a slot `refuse_slot` accepted to the realization `mach dep verify` checks |

| Functions | |
| :--- | :--- |
| [`own_work_tree`](#own_work_tree) | whether `dep_full` is a checkout of its own, holding its own `.git` |
| [`revision`](#revision) | the commit a dependency's checkout is at |
| [`selection_commit`](#selection_commit) | the commit the fixed selector `ref` picks in the checkout at dep_full: a commit or a tag as named, and a branch at dep/\<id>'s pin unless `advance` moves it, then at its fetched tip |
| [`manifest_at`](#manifest_at) | the manifest the commit `rev` of the repository at `dir` holds, allocated from `alloc` |
| [`release_at_pin`](#release_at_pin) | the release dep/\<id>'s pin is tagged with, read from its checkout's refs, none when no release tag points at it |
| [`has_tags`](#has_tags) | whether the checkout at `dir` holds any tag: one that holds none, as a shallow clone (`git submodule update --depth 1`) does, cannot say which release a commit is |
| [`holds_tags`](#holds_tags) | whether the checkout at dep_full holds any tag, which a shallow clone may not |
| [`fetch_selection_tags`](#fetch_selection_tags) | fetch the tags of dep/\<id>'s checkout when it lacks the one its selection reads: the tag a `tag/` ref names, or a release tag naming the pin of a version range; whether it fetched |
| [`pinned_at_release`](#pinned_at_release) | whether dep/\<id> is pinned at the release `version` and its checkout is at that pin; a checkout drifted from its gitlink is not, whatever release it holds, and a directory that is no checkout of its own is not |
| [`release_at`](#release_at) | the release the commit `rev` of the repository at `dir` is tagged with, read from its local refs, none when no `v`-prefixed semver tag points at it |
| [`acquire`](#acquire) | bring dep/\<id> to an acquisition: its slot is observed, refused where realizing it would replace or record work mach did not make, and realized |
| [`fetch_and_checkout`](#fetch_and_checkout) | check dep/\<id>'s checkout out at the selector `ref` after fetching every ref; `offline` fetches nothing, so a selector the checkout does not hold is refused |
| [`gitlink_recorded`](#gitlink_recorded) | whether a moved checkout of dep/\<id> is to be staged: always in a repository root, where the gitlink is the pin, and in a subproject exactly when the enclosing repository already records a gitlink for it |

---

## Slot

```mach
pub rec Slot {
    tree:       u8;
    top:        str;
    record:     str;
    gitlink:    opt[str];
    registered: bool;
    store:      bool;
}
```

a git dependency's slot: what dep/\<id> holds, and its record in the enclosing
repository. the record lives at `top`, the repository's top level where
.gitmodules is, under the path `record`: dep/\<id> when the project root is the
repository root, and the project's prefix ahead of it when the project is a
subdirectory of a repository, so a committed gitlink under a subproject is
its pin exactly as the root's is. `top` is empty when the root is in no work
tree, and then there is no record to read

### slot_dnit

```mach
pub fun slot_dnit(alloc: *std.allocator.Allocator, slot: *Slot)
```

free what an observed slot owns and leave it empty

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator that owns the slot's strings |
| `slot` | the slot to release |

### observe_slot

```mach
pub fun observe_slot(s: *mach.session.Session, root: str, id: str, mode: u8) res[Slot, mach.base.fail.Fail]
```

observe dep/\<id>: what the directory holds and, when the project records pins, its
registration in .gitmodules, its module store and its committed gitlink

| Parameter | |
| :--- | :--- |
| `s` | the session, whose allocator owns the slot's strings |
| `root` | the project root |
| `id` | the dependency identity |
| `mode` | how the project records pins, a pin.mach mode |
| **returns** | the slot, freed with slot_dnit, err when the directory or the repository cannot be read |

### realize_slot

```mach
pub fun realize_slot(s: *mach.session.Session, root: str, id: str, chain: str, url: str, ref: str, mode: u8,
    slot: *Slot, offline: bool, rep: *mach.package.report.Report) res[u8, mach.base.fail.Fail]
```

bring a slot `refuse_slot` accepted to the realization `mach dep verify` checks. a pin its
checkout does not hold is fetched, or refused when `offline`. offline, an uninitialized gitlink
is initialized only from a module store that holds its pin, a subproject's dependency is never
cloned, and a dependency with no gitlink is added only over a checkout or store holding its selector

## Functions

### own_work_tree

```mach
pub fun own_work_tree(alloc: *std.allocator.Allocator, dep_full: str) res[bool, mach.base.fail.Fail]
```

whether `dep_full` is a checkout of its own, holding its own `.git`

### revision

```mach
pub fun revision(s: *mach.session.Session, dep_full: str) res[str, mach.base.fail.Fail]
```

the commit a dependency's checkout is at

| Parameter | |
| :--- | :--- |
| `s` | the session, whose allocator holds the answer |
| `dep_full` | the dependency directory |
| **returns** | HEAD's commit id, owned by the session's allocator, err when the directory is not a checkout of its own or git fails |

### selection_commit

```mach
pub fun selection_commit(s: *mach.session.Session, root: str, id: str, dep_full: str, ref: str,
    advance: bool, offline: bool) res[str, mach.base.fail.Fail]
```

the commit the fixed selector `ref` picks in the checkout at dep_full: a commit or a tag as
named, and a branch at dep/\<id>'s pin unless `advance` moves it, then at its fetched tip. a
revision the checkout lacks is fetched, which `offline` refuses; the caller frees the result

### manifest_at

```mach
pub fun manifest_at(s: *mach.session.Session, alloc: *std.allocator.Allocator, dir: str, rev: str) res[mach.manifest.load.Doc, mach.base.fail.Fail]
```

the manifest the commit `rev` of the repository at `dir` holds, allocated from `alloc`

### release_at_pin

```mach
pub fun release_at_pin(s: *mach.session.Session, root: str, id: str, dep_full: str, fetch: bool) res[opt[mach.package.source.Release], mach.base.fail.Fail]
```

the release dep/\<id>'s pin is tagged with, read from its checkout's refs, none when no release
tag points at it. the highest wins, and the caller frees it with `package_source.release_free`.
with `fetch`, a checkout no release tag of which names the pin fetches its tags first

### has_tags

```mach
pub fun has_tags(a: *std.allocator.Allocator, gi: *mach.package.git.Inspector, dir: str) res[bool, mach.base.fail.Fail]
```

whether the checkout at `dir` holds any tag: one that holds none, as a shallow clone
(`git submodule update --depth 1`) does, cannot say which release a commit is

### holds_tags

```mach
pub fun holds_tags(s: *mach.session.Session, dep_full: str) res[bool, mach.base.fail.Fail]
```

whether the checkout at dep_full holds any tag, which a shallow clone may not

### fetch_selection_tags

```mach
pub fun fetch_selection_tags(s: *mach.session.Session, root: str, id: str, dep_full: str, ref: str,
    ranged: bool, offline: bool) res[bool, mach.base.fail.Fail]
```

fetch the tags of dep/\<id>'s checkout when it lacks the one its selection reads: the tag a
`tag/` ref names, or a release tag naming the pin of a version range; whether it fetched.
`offline` fetches nothing

### pinned_at_release

```mach
pub fun pinned_at_release(s: *mach.session.Session, root: str, id: str, dep_full: str, version: str) res[bool, mach.base.fail.Fail]
```

whether dep/\<id> is pinned at the release `version` and its checkout is at that pin; a
checkout drifted from its gitlink is not, whatever release it holds, and a directory that
is no checkout of its own is not

### release_at

```mach
pub fun release_at(a: *std.allocator.Allocator, gi: *mach.package.git.Inspector, dir: str, rev: str) res[opt[mach.package.source.Release], mach.base.fail.Fail]
```

the release the commit `rev` of the repository at `dir` is tagged with, read from its local
refs, none when no `v`-prefixed semver tag points at it. the highest wins, and the caller
frees it with `package_source.release_free`

### acquire

```mach
pub fun acquire(s: *mach.session.Session, q: *mach.package.source.Acquisition, rep: *mach.package.report.Report) res[u8, mach.base.fail.Fail]
```

bring dep/\<id> to an acquisition: its slot is observed, refused where realizing it would
replace or record work mach did not make, and realized

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `q` | the acquisition |
| `rep` | receives the notes naming what a failure left behind |
| **returns** | what changed, a source.mach REALIZED_* value |

### fetch_and_checkout

```mach
pub fun fetch_and_checkout(s: *mach.session.Session, id: str, dep_full: str, ref: str, offline: bool, rep: *mach.package.report.Report) err[mach.base.fail.Fail]
```

check dep/\<id>'s checkout out at the selector `ref` after fetching every ref; `offline` fetches
nothing, so a selector the checkout does not hold is refused

### gitlink_recorded

```mach
pub fun gitlink_recorded(s: *mach.session.Session, root: str, id: str, mode: u8) res[bool, mach.base.fail.Fail]
```

whether a moved checkout of dep/\<id> is to be staged: always in a repository
root, where the gitlink is the pin, and in a subproject exactly when the
enclosing repository already records a gitlink for it

