[mach](../README.md) / [package](README.md) / **edit**

# mach.package.edit

edit: the commands that change a project's dependencies: add and remove edit the
manifest's [dep] tables, update moves selectors and pins. each is a transaction over the
project root held open: the manifest is published only after the closure it declares is
realized, and a failure names, through the report's effects, every change it leaves behind

<sub>source: [`src/package/edit.mach`](../../../src/package/edit.mach) · imported by 2 modules</sub>

<sub>`A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md) · `package_closure` is [`mach.package.closure`](closure.md) · `manifest_edit` is [`mach.manifest.edit`](../manifest/edit.md) · `session` is [`mach.session`](../session.md) · `report` is [`mach.package.report`](report.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Edit`](#edit) | `rec` | the project root held open for an edit: the manifest is replaced through a sibling temporary and the dependency root is reached through it |

| On `Edit` | |
| :--- | :--- |
| [`begin`](#begin) | |
| [`finish`](#finish) | release the project root; err when closing it fails |
| [`apply`](#apply) | add or remove a declaration within an open edit |

| Functions | |
| :--- | :--- |
| [`start`](#start) | start recording pins for a new project at `root`, as `mach init` does before it adds the project's dependencies; a root that records them already is left as it is |
| [`update`](#update) | `mach dep update`: move one root-declared or transitive identity, or every one under `all` |
| [`update_apply`](#update_apply) | update: the fixed git selectors move first (a ref is fetched and checked out), so their manifests are the closure's when the version ranges resolve; then every chosen release is realized and the closure is pulled, which syncs every path copy once and reports it |
| [`notes`](#notes) | a resolution's notes, as a command shows them unless --quiet |
| [`add_release`](#add_release) | declare a git dependency at the caret range of the release resolution picks for the running compiler and realize the closure, as `mach dep add <root> <name> --git <url>` does |
| [`change`](#change) | add or remove the [dep] table `spec` describes and realize the closure it leaves |

---

## Edit

```mach
pub rec Edit {
    a:       *A.Allocator;
    root:    str;
    root_fd: usize;
}
```

the project root held open for an edit: the manifest is replaced through a sibling
temporary and the dependency root is reached through it

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*A.Allocator` | holds the text of a refusal |
| `root` | `str` | the project root, as a refusal names it |
| `root_fd` | `usize` | the project root held open |

### begin

```mach
pub fun begin(edit: *Edit, a: *A.Allocator, root: str) err[fail.Fail]
```

### finish

```mach
pub fun finish(edit: *Edit) err[fail.Fail]
```

release the project root; err when closing it fails

### apply

```mach
pub fun apply(edit: *Edit, op: *package_closure.Operation, mode: u8, spec: *manifest_edit.DepTableSpec, name: str, adding: bool,
    purge: bool, quiet: bool, realize: bool, offline: bool) err[fail.Fail]
```

add or remove a declaration within an open edit. realize false only declares: the manifest
gains the table (a git one at its resolved range) and nothing is checked out

## Functions

### start

```mach
pub fun start(s: *session.Session, root: str) err[fail.Fail]
```

start recording pins for a new project at `root`, as `mach init` does before it adds the
project's dependencies; a root that records them already is left as it is

### update

```mach
pub fun update(s: *session.Session, a: *A.Allocator, root: str, all: bool, name: str, quiet: bool, lowest: bool, offline: bool,
    rep: *report.Report) err[fail.Fail]
```

`mach dep update`: move one root-declared or transitive identity, or every one under `all`

| Parameter | |
| :--- | :--- |
| `s` | the session; a failure may borrow from it, so the caller shows it before ending it |
| `a` | the storage the operation works in |
| `root` | the project root directory |
| `all` | move every identity |
| `name` | the identity to move when not `all` |
| `quiet` | report no progress lines |
| `lowest` | resolve each range to its lowest admitted release |
| `offline` | fetch nothing |
| `rep` | receives progress, notes, and the effects a failure left |
| **returns** | err naming the failure |

### update_apply

```mach
pub fun update_apply(op: *package_closure.Operation, mode: u8, all: bool, name: str, quiet: bool, lowest: bool,
    offline: bool, closure: *Vector[package_closure.Request]) err[fail.Fail]
```

update: the fixed git selectors move first (a ref is fetched and checked out), so their
manifests are the closure's when the version ranges resolve; then every chosen release is
realized and the closure is pulled, which syncs every path copy once and reports it

`closure` receives the closure the final pull realized, which the caller frees

### notes

```mach
pub fun notes(rep: *report.Report, lines: *Vector[str])
```

a resolution's notes, as a command shows them unless --quiet

### add_release

```mach
pub fun add_release(s: *session.Session, a: *A.Allocator, root: str, name: str, url: str, quiet: bool, realize: bool,
    rep: *report.Report) err[fail.Fail]
```

declare a git dependency at the caret range of the release resolution picks for the running
compiler and realize the closure, as `mach dep add <root> <name> --git <url>` does

| Parameter | |
| :--- | :--- |
| `s` | the session; a failure may borrow from it, so the caller shows it before ending it |
| `a` | the storage the operation works in |
| `root` | the project root directory |
| `name` | the dependency identity |
| `url` | its git source |
| `quiet` | report no progress lines |
| `realize` | check the release out; false only resolves the range and writes the table |
| `rep` | receives progress, notes, and the effects a failure left |
| **returns** | err naming the failure |

### change

```mach
pub fun change(s: *session.Session, a: *A.Allocator, root: str, spec: *manifest_edit.DepTableSpec, name: str, adding: bool,
    purge: bool, quiet: bool, realize: bool, offline: bool, rep: *report.Report) err[fail.Fail]
```

add or remove the [dep] table `spec` describes and realize the closure it leaves

| Parameter | |
| :--- | :--- |
| `s` | the session; a failure may borrow from it, so the caller shows it before ending it |
| `a` | the storage the operation works in |
| `root` | the project root directory |
| `spec` | the table to add, or the name to remove |
| `name` | the dependency identity |
| `adding` | add the table; false removes it |
| `purge` | on remove, delete dep/&lt;name> as well |
| `quiet` | report no progress lines |
| `realize` | realize the closure; false only writes the table |
| `offline` | fetch nothing |
| `rep` | receives progress, notes, and the effects a failure left |
| **returns** | err naming the failure |

