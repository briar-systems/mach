# mach.lang.package.edit

edit: the commands that change a project's dependencies: add and remove edit the
manifest's [dep] tables, update moves selectors and pins. each is a transaction over the
project root held open: the manifest is published only after the closure it declares is
realized, and a failure names, through the report's effects, every change it leaves behind

## rec Edit

```mach
pub rec Edit;
```

the project root held open for an edit: the manifest is replaced through a sibling
temporary and the dependency root is reached through it

## fun begin

```mach
pub fun begin(edit: *Edit, root: str) err[fail.Fail];
```

## fun finish

```mach
pub fun finish(edit: *Edit) err[fail.Fail];
```

release the project root; err when closing it fails

## fun start

```mach
pub fun start(s: *session.Session, root: str) err[fail.Fail];
```

start recording pins for a new project at `root`, as `mach init` does before it adds the
project's dependencies; a root that records them already is left as it is

## fun update

```mach
pub fun update(s: *session.Session, a: *A.Allocator, root: str, all: bool, name: str, quiet: bool, lowest: bool, offline: bool,
rep: *report.Report) err[fail.Fail];
```

`mach dep update`: move one root-declared or transitive identity, or every one under `all`

s: the session; a failure may borrow from it, so the caller shows it before ending it
a: the storage the operation works in
root: the project root directory
all: move every identity
name: the identity to move when not `all`
quiet: report no progress lines
lowest: resolve each range to its lowest admitted release
offline: fetch nothing
rep: receives progress, notes, and the effects a failure left
ret: err naming the failure

## fun update_apply

```mach
pub fun update_apply(op: *package_closure.Operation, mode: u8, all: bool, name: str, quiet: bool, fail_after: i32,
lowest: bool, offline: bool, closure: *Vector[package_closure.Request]) err[fail.Fail];
```

update: the fixed git selectors move first (a ref is fetched and checked out), so their
manifests are the closure's when the version ranges resolve; then every chosen release is
realized and the closure is pulled, which syncs every path copy once and reports it

`closure` receives the closure the final pull realized, which the caller frees;
`fail_after` stops the update with a failure after that many of its steps, 0 for none

## fun notes

```mach
pub fun notes(rep: *report.Report, lines: *Vector[str]);
```

a resolution's notes, as a command shows them unless --quiet

## fun realize_picks

```mach
pub fun realize_picks(op: *package_closure.Operation, mode: u8, picks: *Vector[resolver.Choice], quiet: bool,
offline: bool) err[fail.Fail];
```

move every chosen release's slot to its tag: a missing slot is acquired, and one pinned
elsewhere is checked out and its pin recorded

## fun add_release

```mach
pub fun add_release(s: *session.Session, a: *A.Allocator, root: str, name: str, url: str, quiet: bool, realize: bool,
rep: *report.Report) err[fail.Fail];
```

declare a git dependency at the caret range of the release resolution picks for the running
compiler and realize the closure, as `mach dep add <root> <name> --git <url>` does

s: the session; a failure may borrow from it, so the caller shows it before ending it
a: the storage the operation works in
root: the project root directory
name: the dependency identity
url: its git source
quiet: report no progress lines
realize: check the release out; false only resolves the range and writes the table
rep: receives progress, notes, and the effects a failure left
ret: err naming the failure

## fun change

```mach
pub fun change(s: *session.Session, a: *A.Allocator, root: str, spec: *manifest.DepTableSpec, name: str, adding: bool,
purge: bool, quiet: bool, realize: bool, offline: bool, rep: *report.Report) err[fail.Fail];
```

add or remove the [dep] table `spec` describes and realize the closure it leaves

s: the session; a failure may borrow from it, so the caller shows it before ending it
a: the storage the operation works in
root: the project root directory
spec: the table to add, or the name to remove
name: the dependency identity
adding: add the table; false removes it
purge: on remove, delete dep/<name> as well
quiet: report no progress lines
realize: realize the closure; false only writes the table
offline: fetch nothing
rep: receives progress, notes, and the effects a failure left
ret: err naming the failure

## fun apply

```mach
pub fun apply(edit: *Edit, op: *package_closure.Operation, mode: u8, spec: *manifest.DepTableSpec, name: str, adding: bool,
purge: bool, quiet: bool, realize: bool, offline: bool, fail_after: i32) err[fail.Fail];
```

add or remove a declaration within an open edit. realize false only declares: the manifest
gains the table (a git one at its resolved range) and nothing is checked out. `fail_after`
stops the change with a failure after that many of its steps, 0 for none

## fun purge_beneath

```mach
pub fun purge_beneath(root_fd: usize, name: str) res[usize, fail.Fail];
```

