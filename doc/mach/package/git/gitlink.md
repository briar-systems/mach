[mach](../../README.md) / [package](../README.md) / [git](README.md) / **gitlink**

# mach.package.git.gitlink

gitlink: the git member of the pin contract. a dependency's pin is the gitlink the
enclosing repository stages at dep/&lt;id>, registered in .gitmodules with its source url;
a project in a subdirectory of a repository records its pins under its prefix

<sub>source: [`src/package/git/gitlink.mach`](../../../../src/package/git/gitlink.mach) · imported by 2 modules</sub>

<sub>`session` is [`mach.session`](../../session.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `git` is [`mach.package.git`](../git.md) · `report` is [`mach.package.report`](../report.md)</sub>

| Functions | |
| :--- | :--- |
| [`tracked`](#tracked) | whether `dir` lies inside a Git work tree; false for a plain directory |
| [`begin`](#begin) | |
| [`register`](#register) | record a gitlink the index already holds in .gitmodules from the manifest's source; `top` is the repository's top level and `rel` the gitlink's path from there |
| [`recording_mode`](#recording_mode) · [`read`](#read) | |
| [`read_at`](#read_at) | the gitlink the tree of the commit `rev` in the repository at `dir` records for dep/&lt;id> |
| [`drop`](#drop) · [`record`](#record) | |

---

## Functions

### tracked

```mach
pub fun tracked(s: *session.Session, dir: str) res[bool, fail.Fail]
```

whether `dir` lies inside a Git work tree; false for a plain directory

### begin

```mach
pub fun begin(s: *session.Session, root: str) err[fail.Fail]
```

### register

```mach
pub fun register(s: *session.Session, gi: *git.Inspector, root: str, id: str, rel: str, url: str,
    extra_env: **u8, rep: *report.Report) err[fail.Fail]
```

record a gitlink the index already holds in .gitmodules from the manifest's source;
`top` is the repository's top level and `rel` the gitlink's path from there

### recording_mode

```mach
pub fun recording_mode(s: *session.Session, root: str) res[u8, fail.Fail]
```

### read

```mach
pub fun read(s: *session.Session, root: str, id: str) res[opt[str], fail.Fail]
```

### read_at

```mach
pub fun read_at(s: *session.Session, dir: str, rev: str, id: str) res[opt[str], fail.Fail]
```

the gitlink the tree of the commit `rev` in the repository at `dir` records for dep/&lt;id>

### drop

```mach
pub fun drop(s: *session.Session, root: str, id: str, mode: u8, rep: *report.Report) err[fail.Fail]
```

### record

```mach
pub fun record(s: *session.Session, root: str, id: str) err[fail.Fail]
```

