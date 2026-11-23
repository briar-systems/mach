[mach](../../README.md) / [package](../README.md) / [git](README.md) / **gitlink**

# mach.package.git.gitlink

gitlink: the git member of the pin contract. a dependency's pin is the gitlink the
enclosing repository stages at dep/\<id>, registered in .gitmodules with its source url;
a project in a subdirectory of a repository records its pins under its prefix

Source: [`src/package/git/gitlink.mach`](../../../../src/package/git/gitlink.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.package.git`](../git.md), [`mach.package.report`](../report.md), [`mach.session`](../../session.md).

| Functions | |
| :--- | :--- |
| [`tracked`](#tracked) | whether `dir` lies inside a Git work tree; false for a plain directory |
| [`begin`](#begin) | start recording pins at a root: `git init` there unless it already lies inside a work tree |
| [`register`](#register) | record a gitlink the index already holds in .gitmodules from the manifest's source; `root` is the repository's top level and `rel` the gitlink's path from there |
| [`recording_mode`](#recording_mode) | how the project at a root records pins |
| [`read`](#read) | the gitlink the index of the repository at a root stages for dep/\<id> |
| [`read_at`](#read_at) | the gitlink the tree of the commit `rev` in the repository at `dir` records for dep/\<id> |
| [`drop`](#drop) | stop recording dep/\<id>'s pin: untrack its gitlink and remove its .gitmodules section, keeping the checkout |
| [`record`](#record) | stage what dep/\<id> holds as its gitlink |

---

## Functions

### tracked

```mach
pub fun tracked(s: *mach.session.Session, dir: str) res[bool, mach.base.fail.Fail]
```

whether `dir` lies inside a Git work tree; false for a plain directory

### begin

```mach
pub fun begin(s: *mach.session.Session, root: str) err[mach.base.fail.Fail]
```

start recording pins at a root: `git init` there unless it already lies inside a work tree

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `root` | the project root |
| **returns** | err when git cannot be found or fails |

### register

```mach
pub fun register(s: *mach.session.Session, gi: *mach.package.git.Inspector, root: str, id: str, rel: str, url: str,
    extra_env: **u8, rep: *mach.package.report.Report) err[mach.base.fail.Fail]
```

record a gitlink the index already holds in .gitmodules from the manifest's source;
`root` is the repository's top level and `rel` the gitlink's path from there

### recording_mode

```mach
pub fun recording_mode(s: *mach.session.Session, root: str) res[u8, mach.base.fail.Fail]
```

how the project at a root records pins

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `root` | the project root |
| **returns** | REPOSITORY_ROOT for a root holding `.git` at the top of its work tree, SUBPROJECT for any other root, one with no `.git` entry included, err when `.git` cannot be inspected, a root holding it is not inside a git repository or git fails |

### read

```mach
pub fun read(s: *mach.session.Session, root: str, id: str) res[opt[str], mach.base.fail.Fail]
```

the gitlink the index of the repository at a root stages for dep/\<id>

| Parameter | |
| :--- | :--- |
| `s` | the session, whose allocator owns the answer |
| `root` | the project root, where git runs |
| `id` | the dependency identity |
| **returns** | the pinned commit id, none when no gitlink is staged, err when git fails or its output is malformed |

### read_at

```mach
pub fun read_at(s: *mach.session.Session, dir: str, rev: str, id: str) res[opt[str], mach.base.fail.Fail]
```

the gitlink the tree of the commit `rev` in the repository at `dir` records for dep/\<id>

### drop

```mach
pub fun drop(s: *mach.session.Session, root: str, id: str, mode: u8, rep: *mach.package.report.Report) err[mach.base.fail.Fail]
```

stop recording dep/\<id>'s pin: untrack its gitlink and remove its .gitmodules section,
keeping the checkout. a subproject, or a root that stages no gitlink for it, changes nothing

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `root` | the project root |
| `id` | the dependency identity |
| `mode` | how the project records pins, a pin.mach mode |
| `rep` | receives a note naming what a failure left |
| **returns** | err when .gitmodules has unstaged changes or misnames the path, or git fails |

### record

```mach
pub fun record(s: *mach.session.Session, root: str, id: str) err[mach.base.fail.Fail]
```

stage what dep/\<id> holds as its gitlink

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `root` | the project root, where git runs |
| `id` | the dependency identity |
| **returns** | err when git fails |

