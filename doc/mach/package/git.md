[mach](../README.md) / [package](README.md) / **git**

# mach.package.git

git: the one outside tool mach runs. a dependency is acquired and pinned through git
(package/git holds the source and pin members built on it); nothing else in the
toolchain spawns it, and a build never does. every git process starts here, with one
environment that keeps the user's configuration and drops whatever would redirect git to
another repository, and every failure names the command, how it ended and what git printed

<sub>source: [`src/package/git.mach`](../../../src/package/git.mach) · imported by 8 modules</sub>

<sub>`exec` is `std.process.exec` · `A` is `std.allocator` · `host_environment` is [`mach.host.environment`](../host/environment.md) · `Tree` is private · `fail` is [`mach.base.fail`](../base/fail.md) · `session` is [`mach.session`](../session.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Entry`](#entry) | `rec` |  |
| [`Inspector`](#inspector) | `rec` | an inspector spawns git with one environment and answers every committed-mode question about a checkout from one tree listing, so verification costs a constant number of git processes per checkout rather than one per walked path |
| [`Bytes`](#bytes) | `rec` |  |

| On `Inspector` | |
| :--- | :--- |
| [`inspector_init`](#inspector_init) | an inspector whose git processes inherit `environ`, a session's environment |
| [`inspector_dnit`](#inspector_dnit) · [`query_capture`](#query_capture) · [`query`](#query) · [`query_raw`](#query_raw) · [`op`](#op) | |
| [`path_mode`](#path_mode) | the committed mode of `<prefix><rel>` at HEAD, or "" when HEAD has no such entry |
| [`inspector_pending`](#inspector_pending) | an inspector that resolves git on first use, its processes inheriting `environ` |
| [`inspector_require`](#inspector_require) | start a pending inspector; one already started is left as it is |
| [`query_named`](#query_named) · [`require_root`](#require_root) · [`project_root_authority`](#project_root_authority) · [`declaring_prefix`](#declaring_prefix) · [`index_entry_field`](#index_entry_field) · [`require_matches_index`](#require_matches_index) · [`require_tracked`](#require_tracked) | |
| [`is_work_tree`](#is_work_tree) | whether `dir` lies inside a git work tree: git answers by its exit, so a refusing exit is false, and a failure to run git or read its answer is the failure |

| On `Bytes` | |
| :--- | :--- |
| [`bytes_dnit`](#bytes_dnit) | |

| Functions | |
| :--- | :--- |
| [`count_nil_terminated`](#count_nil_terminated) · [`object_id_valid`](#object_id_valid) | |

| Constants | |
| :--- | :--- |
| [`AUTHORITY_REPO`](#constants) · [`AUTHORITY_NESTED`](#constants) | |

---

## Entry

```mach
pub rec Entry {
    path: str;
    mode: str;
}
```

## Inspector

```mach
pub rec Inspector {
    program:       str;
    inherited_env: exec.Environment;
    alloc:         *A.Allocator;
    env_ready:     bool;
    env_inherited: exec.Environment;
    env:           host_environment.Strings;
    trees:         Vector[Tree];
    trees_ready:   bool;
}
```

an inspector spawns git with one environment and answers every committed-mode question
about a checkout from one tree listing, so verification costs a constant number of git
processes per checkout rather than one per walked path. a command that can move
a checkout drops the listings.

### inspector_init

```mach
pub fun inspector_init(alloc: *A.Allocator, environ: exec.Environment) res[Inspector, fail.Fail]
```

an inspector whose git processes inherit `environ`, a session's environment

### inspector_dnit

```mach
pub fun inspector_dnit(alloc: *A.Allocator, gi: *Inspector)
```

### query_capture

```mach
pub fun query_capture(alloc: *A.Allocator, gi: *Inspector, dir: str, args: *str, count: usize,
    trim_line_end: bool, extra_env: **u8, stdin_fd: usize, capture_stderr: bool) res[Bytes, fail.Fail]
```

### query

```mach
pub fun query(alloc: *A.Allocator, gi: *Inspector, dir: str, args: *str, count: usize) res[str, fail.Fail]
```

### query_raw

```mach
pub fun query_raw(alloc: *A.Allocator, gi: *Inspector, dir: str, args: *str, count: usize) res[str, fail.Fail]
```

### op

```mach
pub fun op(alloc: *A.Allocator, gi: *Inspector, dir: str, args: *str, count: usize, extra_env: **u8) res[str, fail.Fail]
```

### path_mode

```mach
pub fun path_mode(s: *session.Session, gi: *Inspector, alias: str,
    git_dir: str, git_prefix: str, rel: str) res[str, fail.Fail]
```

the committed mode of `<prefix><rel>` at HEAD, or "" when HEAD has no such entry

### inspector_pending

```mach
pub fun inspector_pending(environ: exec.Environment) Inspector
```

an inspector that resolves git on first use, its processes inheriting `environ`

### inspector_require

```mach
pub fun inspector_require(alloc: *A.Allocator, gi: *Inspector) err[fail.Fail]
```

start a pending inspector; one already started is left as it is

### query_named

```mach
pub fun query_named(s: *session.Session, gi: *Inspector, alias: str, dir: str,
    args: *str, count: usize, fact: str) res[str, fail.Fail]
```

### require_root

```mach
pub fun require_root(s: *session.Session, gi: *Inspector, alias: str, dir: str,
    subject: str) err[fail.Fail]
```

### project_root_authority

```mach
pub fun project_root_authority(s: *session.Session, gi: *Inspector, alias: str,
    project_root: str) res[u8, fail.Fail]
```

### declaring_prefix

```mach
pub fun declaring_prefix(s: *session.Session, gi: *Inspector, alias: str, dir: str) res[str, fail.Fail]
```

### index_entry_field

```mach
pub fun index_entry_field(s: *session.Session, gi: *Inspector, alias: str, root: str,
    rel: str, field: usize, fact: str) res[str, fail.Fail]
```

### require_matches_index

```mach
pub fun require_matches_index(s: *session.Session, gi: *Inspector, alias: str, root: str,
    rel: str, subject: str) err[fail.Fail]
```

### require_tracked

```mach
pub fun require_tracked(s: *session.Session, gi: *Inspector, alias: str, root: str,
    rel: str, subject: str) err[fail.Fail]
```

### is_work_tree

```mach
pub fun is_work_tree(alloc: *A.Allocator, gi: *Inspector, dir: str) res[bool, fail.Fail]
```

whether `dir` lies inside a git work tree: git answers by its exit, so a refusing exit is
false, and a failure to run git or read its answer is the failure

## Bytes

```mach
pub rec Bytes {
    data: *u8;
    len:  usize;
}
```

### bytes_dnit

```mach
pub fun bytes_dnit(alloc: *A.Allocator, bytes: *Bytes)
```

## Functions

### count_nil_terminated

```mach
pub fun count_nil_terminated(arr: **u8) usize
```

### object_id_valid

```mach
pub fun object_id_valid(id: str) bool
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `AUTHORITY_REPO` | `u8` | `0` |  |
| `AUTHORITY_NESTED` | `u8` | `1` |  |

