# mach.lang.package.git

git: the one outside tool mach runs. a dependency is acquired and pinned through git
(package/git holds the source and pin members built on it); nothing else in the
toolchain spawns it, and a build never does. every git process starts here, with one
environment that keeps the user's configuration and drops whatever would redirect git to
another repository, and every failure names the command, how it ended and what git printed

## rec Entry

```mach
pub rec Entry;
```

## rec Tree

```mach
pub rec Tree;
```

one checked-out tree's committed entries, as `git ls-tree -r -t --full-tree HEAD` lists
them: repository-relative paths with their object modes, sorted by path

## rec Inspector

```mach
pub rec Inspector;
```

an inspector spawns git with one environment and answers every committed-mode question
about a checkout from one tree listing, so verification costs a constant number of git
processes per checkout rather than one per walked path. a command that can move
a checkout drops the listings.

## fun inspector_init

```mach
pub fun inspector_init(alloc: *A.Allocator, environ: exec.Environment) res[Inspector, fail.Fail];
```

an inspector whose git processes inherit `environ`, a session's environment

## fun inspector_dnit

```mach
pub fun inspector_dnit(alloc: *A.Allocator, gi: *Inspector);
```

## rec Bytes

```mach
pub rec Bytes;
```

## fun bytes_free

```mach
pub fun bytes_free(alloc: *A.Allocator, bytes: *Bytes);
```

## fun count_nil_terminated

```mach
pub fun count_nil_terminated(arr: **u8) usize;
```

## fun env_prefix

```mach
pub fun env_prefix(alloc: *A.Allocator, entry: *Unit, prefix: str) res[bool, fail.Fail];
```

whether the name of `entry`, NAME=value in native units, begins with the
segments of `prefix` by the host's environment name identity. a prefix ending
in '=' names the whole variable

## fun query_capture

```mach
pub fun query_capture(alloc: *A.Allocator, gi: *Inspector, dir: str, args: *str, count: usize,
trim_line_end: bool, extra_env: **u8, stdin_fd: usize, capture_stderr: bool) res[Bytes, fail.Fail];
```

## fun query

```mach
pub fun query(alloc: *A.Allocator, gi: *Inspector, dir: str, args: *str, count: usize) res[str, fail.Fail];
```

## fun query_raw

```mach
pub fun query_raw(alloc: *A.Allocator, gi: *Inspector, dir: str, args: *str, count: usize) res[str, fail.Fail];
```

## fun op

```mach
pub fun op(alloc: *A.Allocator, gi: *Inspector, dir: str, args: *str, count: usize, extra_env: **u8) res[str, fail.Fail];
```

## fun path_mode

```mach
pub fun path_mode(s: *session.Session, gi: *Inspector, alias: str,
git_dir: str, git_prefix: str, rel: str) res[str, fail.Fail];
```

the committed mode of `<prefix><rel>` at HEAD, or "" when HEAD has no such entry

## fun inspector_pending

```mach
pub fun inspector_pending(environ: exec.Environment) Inspector;
```

an inspector that resolves git on first use, its processes inheriting `environ`

## fun inspector_require

```mach
pub fun inspector_require(alloc: *A.Allocator, gi: *Inspector) err[fail.Fail];
```

start a pending inspector; one already started is left as it is

## fun query_named

```mach
pub fun query_named(s: *session.Session, gi: *Inspector, alias: str, dir: str,
args: *str, count: usize, fact: str) res[str, fail.Fail];
```

## fun require_root

```mach
pub fun require_root(s: *session.Session, gi: *Inspector, alias: str, dir: str,
subject: str) err[fail.Fail];
```

## val AUTHORITY_REPO

```mach
pub val AUTHORITY_REPO:   u8 = 0
```

## val AUTHORITY_NESTED

```mach
pub val AUTHORITY_NESTED: u8 = 1
```

## fun project_root_authority

```mach
pub fun project_root_authority(s: *session.Session, gi: *Inspector, alias: str,
project_root: str) res[u8, fail.Fail];
```

## fun declaring_prefix

```mach
pub fun declaring_prefix(s: *session.Session, gi: *Inspector, alias: str, dir: str) res[str, fail.Fail];
```

## fun object_id_valid

```mach
pub fun object_id_valid(id: str) bool;
```

## fun index_entry_field

```mach
pub fun index_entry_field(s: *session.Session, gi: *Inspector, alias: str, root: str,
rel: str, field: usize, fact: str) res[str, fail.Fail];
```

## fun require_matches_index

```mach
pub fun require_matches_index(s: *session.Session, gi: *Inspector, alias: str, root: str,
rel: str, subject: str) err[fail.Fail];
```

## fun require_tracked

```mach
pub fun require_tracked(s: *session.Session, gi: *Inspector, alias: str, root: str,
rel: str, subject: str) err[fail.Fail];
```

## fun is_work_tree

```mach
pub fun is_work_tree(alloc: *A.Allocator, gi: *Inspector, dir: str) bool;
```

