[mach](../README.md) / [package](README.md) / **git**

# mach.package.git

git: the one outside tool mach runs. a dependency is acquired and pinned through git
(package/git holds the source and pin members built on it); nothing else in the
toolchain spawns it, and a build never does. every git process starts here, with one
environment that keeps the user's configuration and drops whatever would redirect git to
another repository, and every failure names the command, how it ended and what git printed

Source: [`src/package/git.mach`](../../../src/package/git.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.host.environment`](../host/environment.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Entry`](#entry) | `rec` | one committed entry of a tree listing |
| [`Inspector`](#inspector) | `rec` | an inspector spawns git with one environment and answers every committed-mode question about a checkout from one tree listing, so verification costs a constant number of git processes per checkout rather than one per walked path |
| [`Bytes`](#bytes) | `rec` | a git command's captured output, nul-terminated past `len` |

| On `Inspector` | |
| :--- | :--- |
| [`inspector_init`](#inspector_init) | an inspector whose git processes inherit `environ`, a session's environment |
| [`inspector_dnit`](#inspector_dnit) | release an inspector: its tree listings, its environment and the resolved git path |
| [`query_capture`](#query_capture) | run git in a directory, with fsmonitor and the untracked cache turned off, and capture what it prints |
| [`query`](#query) | run git in a directory and answer its output as text, trailing line ends dropped |
| [`query_raw`](#query_raw) | `query` with the output kept exactly as printed |
| [`op`](#op) | `query` with extra environment entries, for a git command that changes a repository |
| [`path_mode`](#path_mode) | the committed mode of `<prefix><rel>` at HEAD, or "" when HEAD has no such entry |
| [`inspector_pending`](#inspector_pending) | an inspector that resolves git on first use, its processes inheriting `environ` |
| [`inspector_require`](#inspector_require) | start a pending inspector; one already started is left as it is |
| [`query_named`](#query_named) | `query` whose failure names the fact being verified and the dependency |
| [`require_root`](#require_root) | refuse a directory that is not inside a git work tree |
| [`project_root_authority`](#project_root_authority) | whether a project root is a repository's own root or nested in one: a root with no `.git` entry, or one below the top of its work tree, is nested |
| [`declaring_prefix`](#declaring_prefix) | the path of a directory below the top of its work tree, as `git rev-parse --show-prefix` prints it, empty at the top |
| [`index_entry_field`](#index_entry_field) | one space-separated field of a path's index entry, as `git ls-files --stage` prints it |
| [`require_matches_index`](#require_matches_index) | refuse a path whose work-tree state differs from what is staged, an untracked path included |
| [`require_tracked`](#require_tracked) | refuse a path the index does not hold |
| [`ignore_rule`](#ignore_rule) | the ignore rule that ignores `path` in the work tree at `dir`, as git names it (`<source>:<line>:<pattern>`), none when no rule ignores it |
| [`is_work_tree`](#is_work_tree) | whether `dir` lies inside a git work tree: git answers by its exit, so a refusing exit is false, and a failure to run git or read its answer is the failure |

| On `Bytes` | |
| :--- | :--- |
| [`bytes_dnit`](#bytes_dnit) | free captured output and leave it empty |

| Functions | |
| :--- | :--- |
| [`object_id_valid`](#object_id_valid) | whether a text is a full git object id: 40 or 64 lowercase hex digits |

| Constants | |
| :--- | :--- |
| [`AUTHORITY_REPO`](#constants), [`AUTHORITY_NESTED`](#constants) | |

---

## Entry

```mach
pub rec Entry {
    path: str;
    mode: str;
}
```

one committed entry of a tree listing

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `str` | the repository-relative path |
| `mode` | `str` | its object mode as git prints it, such as `100644` or `160000` |

## Inspector

```mach
pub rec Inspector {
    program:       str;
    inherited_env: std.system.os.Environment;
    alloc:         *std.allocator.Allocator;
    env_ready:     bool;
    env_inherited: std.system.os.Environment;
    env:           mach.host.environment.Strings;
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
pub fun inspector_init(alloc: *std.allocator.Allocator, environ: std.system.os.Environment) res[Inspector, mach.base.fail.Fail]
```

an inspector whose git processes inherit `environ`, a session's environment

### inspector_dnit

```mach
pub fun inspector_dnit(alloc: *std.allocator.Allocator, gi: *Inspector)
```

release an inspector: its tree listings, its environment and the resolved git path

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator that owns the git path |
| `gi` | the inspector to release |

### query_capture

```mach
pub fun query_capture(alloc: *std.allocator.Allocator, gi: *Inspector, dir: str, args: *str, count: usize,
    trim_line_end: bool, extra_env: **u8, stdin_fd: usize, capture_stderr: bool) res[Bytes, mach.base.fail.Fail]
```

run git in a directory, with fsmonitor and the untracked cache turned off, and capture what
it prints

| Parameter | |
| :--- | :--- |
| `alloc` | allocates the output, which the caller frees with bytes_dnit |
| `gi` | the inspector whose git and environment run it |
| `dir` | the directory git runs in, passed as `-C` |
| `args` | the subcommand and its arguments |
| `count` | the number of `args` |
| `trim_line_end` | drop the output's trailing line ends |
| `extra_env` | nil-terminated NAME=value entries layered on git's environment, or nil |
| `stdin_fd` | the handle git reads as standard input, or INVALID_HANDLE |
| `capture_stderr` | capture standard error with the output |
| **returns** | the output, err naming the command when it fails to run or exits refusing |

### query

```mach
pub fun query(alloc: *std.allocator.Allocator, gi: *Inspector, dir: str, args: *str, count: usize) res[str, mach.base.fail.Fail]
```

run git in a directory and answer its output as text, trailing line ends dropped

| Parameter | |
| :--- | :--- |
| `alloc` | allocates the text, which the caller owns |
| `gi` | the inspector whose git runs it |
| `dir` | the directory git runs in |
| `args` | the subcommand and its arguments |
| `count` | the number of `args` |
| **returns** | the text, err when git fails or prints a nul byte |

### query_raw

```mach
pub fun query_raw(alloc: *std.allocator.Allocator, gi: *Inspector, dir: str, args: *str, count: usize) res[str, mach.base.fail.Fail]
```

`query` with the output kept exactly as printed

| Parameter | |
| :--- | :--- |
| `alloc` | allocates the text, which the caller owns |
| `gi` | the inspector whose git runs it |
| `dir` | the directory git runs in |
| `args` | the subcommand and its arguments |
| `count` | the number of `args` |
| **returns** | the text, err when git fails or prints a nul byte |

### op

```mach
pub fun op(alloc: *std.allocator.Allocator, gi: *Inspector, dir: str, args: *str, count: usize, extra_env: **u8) res[str, mach.base.fail.Fail]
```

`query` with extra environment entries, for a git command that changes a repository

| Parameter | |
| :--- | :--- |
| `alloc` | allocates the text, which the caller owns |
| `gi` | the inspector whose git runs it |
| `dir` | the directory git runs in |
| `args` | the subcommand and its arguments |
| `count` | the number of `args` |
| `extra_env` | nil-terminated NAME=value entries layered on git's environment, or nil |
| **returns** | the output as text, err when git fails or prints a nul byte |

### path_mode

```mach
pub fun path_mode(s: *mach.session.Session, gi: *Inspector, alias: str,
    git_dir: str, git_prefix: str, rel: str) res[str, mach.base.fail.Fail]
```

the committed mode of `<prefix><rel>` at HEAD, or "" when HEAD has no such entry

### inspector_pending

```mach
pub fun inspector_pending(environ: std.system.os.Environment) Inspector
```

an inspector that resolves git on first use, its processes inheriting `environ`

### inspector_require

```mach
pub fun inspector_require(alloc: *std.allocator.Allocator, gi: *Inspector) err[mach.base.fail.Fail]
```

start a pending inspector; one already started is left as it is

### query_named

```mach
pub fun query_named(s: *mach.session.Session, gi: *Inspector, alias: str, dir: str,
    args: *str, count: usize, fact: str) res[str, mach.base.fail.Fail]
```

`query` whose failure names the fact being verified and the dependency

| Parameter | |
| :--- | :--- |
| `s` | the session, whose allocator holds the answer |
| `gi` | the inspector whose git runs it |
| `alias` | the dependency a failure names |
| `dir` | the directory git runs in |
| `args` | the subcommand and its arguments |
| `count` | the number of `args` |
| `fact` | what the query verifies, as a failure names it |
| **returns** | the text, err when git fails |

### require_root

```mach
pub fun require_root(s: *mach.session.Session, gi: *Inspector, alias: str, dir: str,
    subject: str) err[mach.base.fail.Fail]
```

refuse a directory that is not inside a git work tree

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `gi` | the inspector whose git runs the check |
| `alias` | the dependency a failure names |
| `dir` | the directory checked |
| `subject` | what the check verifies, as a failure to run it names it |
| **returns** | err when git fails or the directory is not in a work tree |

### project_root_authority

```mach
pub fun project_root_authority(s: *mach.session.Session, gi: *Inspector, alias: str,
    project_root: str) res[u8, mach.base.fail.Fail]
```

whether a project root is a repository's own root or nested in one: a root with no `.git`
entry, or one below the top of its work tree, is nested

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `gi` | the inspector, started here when still pending |
| `alias` | the dependency a failure names |
| `project_root` | the project root |
| **returns** | AUTHORITY_REPO or AUTHORITY_NESTED, err when `.git` cannot be inspected or the root is not inside a git repository |

### declaring_prefix

```mach
pub fun declaring_prefix(s: *mach.session.Session, gi: *Inspector, alias: str, dir: str) res[str, mach.base.fail.Fail]
```

the path of a directory below the top of its work tree, as `git rev-parse --show-prefix`
prints it, empty at the top

| Parameter | |
| :--- | :--- |
| `s` | the session, whose allocator holds the answer |
| `gi` | the inspector whose git runs it |
| `alias` | the dependency a failure names |
| `dir` | the directory |
| **returns** | the prefix, err when git fails |

### index_entry_field

```mach
pub fun index_entry_field(s: *mach.session.Session, gi: *Inspector, alias: str, root: str,
    rel: str, field: usize, fact: str) res[str, mach.base.fail.Fail]
```

one space-separated field of a path's index entry, as `git ls-files --stage` prints it

| Parameter | |
| :--- | :--- |
| `s` | the session, whose allocator holds the answer |
| `gi` | the inspector whose git runs it |
| `alias` | the dependency a failure names |
| `root` | the work tree git runs in |
| `rel` | the path, relative to `root` |
| `field` | the zero-based field: 0 the mode, 1 the object id, 2 the stage |
| `fact` | what the query verifies, as a failure names it |
| **returns** | the field, empty when the path has no entry, err when git fails |

### require_matches_index

```mach
pub fun require_matches_index(s: *mach.session.Session, gi: *Inspector, alias: str, root: str,
    rel: str, subject: str) err[mach.base.fail.Fail]
```

refuse a path whose work-tree state differs from what is staged, an untracked path included

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `gi` | the inspector whose git runs it |
| `alias` | the dependency a failure names |
| `root` | the work tree git runs in |
| `rel` | the path, relative to `root` |
| `subject` | what the check verifies, as a failure to run it names it |
| **returns** | err when the path differs from the index or git fails |

### require_tracked

```mach
pub fun require_tracked(s: *mach.session.Session, gi: *Inspector, alias: str, root: str,
    rel: str, subject: str) err[mach.base.fail.Fail]
```

refuse a path the index does not hold

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `gi` | the inspector whose git runs it |
| `alias` | the dependency a failure names |
| `root` | the work tree git runs in |
| `rel` | the path, relative to `root` |
| `subject` | what the check verifies, as a failure to run it names it |
| **returns** | err when the path is not staged or git fails |

### ignore_rule

```mach
pub fun ignore_rule(alloc: *std.allocator.Allocator, gi: *Inspector, dir: str, path: str) res[opt[str], mach.base.fail.Fail]
```

the ignore rule that ignores `path` in the work tree at `dir`, as git names it
(`<source>:<line>:<pattern>`), none when no rule ignores it

### is_work_tree

```mach
pub fun is_work_tree(alloc: *std.allocator.Allocator, gi: *Inspector, dir: str) res[bool, mach.base.fail.Fail]
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

a git command's captured output, nul-terminated past `len`

| Field | Type | |
| :--- | :--- | :--- |
| `data` | `*u8` | the bytes, or nil once released |
| `len` | `usize` | the number of bytes before the terminator |

### bytes_dnit

```mach
pub fun bytes_dnit(alloc: *std.allocator.Allocator, bytes: *Bytes)
```

free captured output and leave it empty

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator that owns the bytes |
| `bytes` | the output to release |

## Functions

### object_id_valid

```mach
pub fun object_id_valid(id: str) bool
```

whether a text is a full git object id: 40 or 64 lowercase hex digits

| Parameter | |
| :--- | :--- |
| `id` | the text |
| **returns** | true for a sha-1 or sha-256 object id |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `AUTHORITY_REPO` | `u8` | `0` |  |
| `AUTHORITY_NESTED` | `u8` | `1` |  |

