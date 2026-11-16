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
| [`begin`](#begin) | |
| [`register`](#register) | record a gitlink the index already holds in .gitmodules from the manifest's source; `top` is the repository's top level and `rel` the gitlink's path from there |
| [`recording_mode`](#recording_mode), [`read`](#read) | |
| [`read_at`](#read_at) | the gitlink the tree of the commit `rev` in the repository at `dir` records for dep/\<id> |
| [`drop`](#drop), [`record`](#record) | |

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

### register

```mach
pub fun register(s: *mach.session.Session, gi: *mach.package.git.Inspector, root: str, id: str, rel: str, url: str,
    extra_env: **u8, rep: *mach.package.report.Report) err[mach.base.fail.Fail]
```

record a gitlink the index already holds in .gitmodules from the manifest's source;
`top` is the repository's top level and `rel` the gitlink's path from there

### recording_mode

```mach
pub fun recording_mode(s: *mach.session.Session, root: str) res[u8, mach.base.fail.Fail]
```

### read

```mach
pub fun read(s: *mach.session.Session, root: str, id: str) res[opt[str], mach.base.fail.Fail]
```

### read_at

```mach
pub fun read_at(s: *mach.session.Session, dir: str, rev: str, id: str) res[opt[str], mach.base.fail.Fail]
```

the gitlink the tree of the commit `rev` in the repository at `dir` records for dep/\<id>

### drop

```mach
pub fun drop(s: *mach.session.Session, root: str, id: str, mode: u8, rep: *mach.package.report.Report) err[mach.base.fail.Fail]
```

### record

```mach
pub fun record(s: *mach.session.Session, root: str, id: str) err[mach.base.fail.Fail]
```

