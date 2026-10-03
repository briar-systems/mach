# mach.lang.package.git.checkout

checkout: the git half of the source contract that realizes dep/<id>. a dependency is
a submodule: its slot is observed (what dep/<id> holds and what the enclosing repository
records for it), refused where realizing it would replace work mach did not make, and
brought to its pin or selector by initializing, checking out, adopting or adding it. a
selector is a `ref` (`branch/`, `tag/` or `commit/`), and a release is read from the
`v`-prefixed tags a checkout holds

## fun own_work_tree

```mach
pub fun own_work_tree(alloc: *A.Allocator, dep_full: str) bool;
```

## fun revision

```mach
pub fun revision(s: *session.Session, dep_full: str) res[str, fail.Fail];
```

## fun selection_commit

```mach
pub fun selection_commit(s: *session.Session, root: str, id: str, dep_full: str, ref: str,
advance: bool, offline: bool) res[str, fail.Fail];
```

the commit the fixed selector `ref` picks in the checkout at dep_full: a commit or a tag as
named, and a branch at dep/<id>'s pin unless `advance` moves it, then at its fetched tip. a
revision the checkout lacks is fetched, which `offline` refuses; the caller frees the result

## fun manifest_at

```mach
pub fun manifest_at(s: *session.Session, alloc: *A.Allocator, dir: str, rev: str) res[manifest.Doc, fail.Fail];
```

the manifest the commit `rev` of the repository at `dir` holds, allocated from `alloc`

## fun release_at_pin

```mach
pub fun release_at_pin(s: *session.Session, root: str, id: str, dep_full: str, fetch: bool) res[str, fail.Fail];
```

the release version dep/<id>'s pin is tagged with, read from its checkout's refs (an owned ""
when no release tag points at it); the highest wins, and the caller frees the result. with
`fetch`, a checkout no release tag of which names the pin fetches its tags first

## fun has_tags

```mach
pub fun has_tags(a: *A.Allocator, gi: *git.Inspector, dir: str) res[bool, fail.Fail];
```

whether the checkout at `dir` holds any tag: one that holds none, as a shallow clone
(`git submodule update --depth 1`) does, cannot say which release a commit is

## fun holds_tags

```mach
pub fun holds_tags(s: *session.Session, dep_full: str) res[bool, fail.Fail];
```

whether the checkout at dep_full holds any tag, which a shallow clone may not

## fun fetch_selection_tags

```mach
pub fun fetch_selection_tags(s: *session.Session, root: str, id: str, dep_full: str, ref: str,
ranged: bool, offline: bool) res[bool, fail.Fail];
```

fetch the tags of dep/<id>'s checkout when it lacks the one its selection reads: the tag a
`tag/` ref names, or a release tag naming the pin of a version range; whether it fetched.
`offline` fetches nothing

## fun pinned_at_release

```mach
pub fun pinned_at_release(s: *session.Session, root: str, id: str, dep_full: str, version: str) bool;
```

whether dep/<id> is pinned at the release `version` and its checkout is at that pin; a
checkout drifted from its gitlink is not, whatever release it holds

## fun release_at

```mach
pub fun release_at(a: *A.Allocator, gi: *git.Inspector, dir: str, rev: str) res[str, fail.Fail];
```

the release version the commit `rev` of the repository at `dir` is tagged with, read from
its local refs (an owned "" when no `v`-prefixed semver tag points at it); the highest
wins, and the caller frees the result on every path

## rec Slot

```mach
pub rec Slot;
```

a git dependency's slot: what dep/<id> holds, and its record in the enclosing
repository. the record lives at `top`, the repository's top level where
.gitmodules is, under the path `record`: dep/<id> when the project root is the
repository root, and the project's prefix ahead of it when the project is a
subdirectory of a repository, so a committed gitlink under a subproject is
its pin exactly as the root's is. `top` is empty when the root is in no work
tree, and then there is no record to read

## fun slot_dnit

```mach
pub fun slot_dnit(alloc: *A.Allocator, slot: *Slot);
```

## fun slot_tree

```mach
pub fun slot_tree(alloc: *A.Allocator, dep_full: str) res[u8, fail.Fail];
```

## fun observe_slot

```mach
pub fun observe_slot(s: *session.Session, root: str, id: str, mode: u8) res[Slot, fail.Fail];
```

## fun refuse_slot

```mach
pub fun refuse_slot(s: *session.Session, root: str, id: str, mode: u8, slot: *Slot, accept: bool) err[fail.Fail];
```

refuse a slot whose realization would replace or record work mach did not make

## fun realize_slot

```mach
pub fun realize_slot(s: *session.Session, root: str, id: str, url: str, ref: str, mode: u8,
slot: *Slot, offline: bool, rep: *report.Report) res[u8, fail.Fail];
```

bring a slot `refuse_slot` accepted to the realization `mach dep verify` checks. a pin its
checkout does not hold is fetched, or refused when `offline`. offline, an uninitialized gitlink
is initialized only from a module store that holds its pin, a subproject's dependency is never
cloned, and a dependency with no gitlink is added only over a checkout or store holding its selector

## fun acquire

```mach
pub fun acquire(s: *session.Session, q: *package_source.Acquisition, rep: *report.Report) res[u8, fail.Fail];
```

bring dep/<id> to an acquisition: its slot is observed, refused where realizing it would
replace or record work mach did not make, and realized

s: the session
q: the acquisition
rep: receives the notes naming what a failure left behind
ret: what changed, a source.mach REALIZED_* value

## fun fetch_and_checkout

```mach
pub fun fetch_and_checkout(s: *session.Session, id: str, dep_full: str, ref: str, offline: bool, rep: *report.Report) err[fail.Fail];
```

check dep/<id>'s checkout out at the selector `ref` after fetching every ref; `offline` fetches
nothing, so a selector the checkout does not hold is refused

## fun gitlink_recorded

```mach
pub fun gitlink_recorded(s: *session.Session, root: str, id: str, mode: u8) res[bool, fail.Fail];
```

whether a moved checkout of dep/<id> is to be staged: always in a repository
root, where the gitlink is the pin, and in a subproject exactly when the
enclosing repository already records a gitlink for it

