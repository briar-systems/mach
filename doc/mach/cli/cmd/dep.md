# mach.cli.cmd.dep

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach dep`: route to one action: list, add, remove, update, pull, verify, or the
deprecated sync, which runs pull after a note

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 success, 1 missing or unknown action, unknown flag, or a failed action, 2 setup failure

## fun pull_project

```mach
pub fun pull_project(root: str, quiet: bool) i64;
```

realize the dependency closure of a project: clone or link every declared dependency
transitively, check each realized manifest's project id against its declared name, and
report realized directories under dep/ that are no longer in the closure

the closure is one walk over every edge kind: the root's declarations, then the
declarations of each realized dependency, whether it was reached by a version range, a
git ref or a path. the root's dep/ holds every identity once, and a version range any
project in the closure declares is pinned there:

- a range's pin is a committed gitlink. for a repository root it is the root's own
  gitlink at dep/<id>; for a subproject, a project in a subdirectory of a repository, it
  is the gitlink the enclosing repository commits under the subproject's prefix. pull
  realizes the gitlink's commit on a fresh clone and restores a checkout that drifted
  from it, and update stages the moved gitlink. a subproject with no gitlink for a
  dependency gets a plain clone, as a project outside any repository does
- a range a dependency declares resolves with the root's own (mach dep add, mach dep
  update), whether the dependency was reached by a ref or a path: when the root holds no
  checkout of the identity yet, it is seeded by the dependency's committed gitlink for it,
  update --all moves it to the highest release every range admits, and a root declaration
  of the same identity overrides it
- a range with neither a gitlink nor a checkout under the root is refused here with
  `run mach dep update <root> <id>`, which pins it whether the root or a dependency
  declares it

root: the project root directory
quiet: suppress progress lines
ret: 0 realized, 1 a manifest, resolution, identity, or checkout error (printed), 2 the
       allocator or session could not be initialised

## fun verify_project

```mach
pub fun verify_project(root: str, release: bool) i64;
```

check that every dependency of a project is realized and consistent without changing
anything; prints "ok" on success. a project root that is not a repository root is noted
and verified from the realized checkouts. a realized directory under dep/ outside the
closure is an error here, where pull reports and retains it

root: the project root directory
release: also refuse a root dependency selected by a branch, a commit or a path
ret: 0 verified, 1 a mismatch or error (printed), 2 the allocator or session could not
         be initialised

## fun pin_command

```mach
pub fun pin_command(a: *A.Allocator, root: str, id: str) str;
```

the command that pins and realizes a version-selected dependency with no checkout yet;
pull's refusal and init's --no-deps hint both name it

## fun unpinned_refusal

```mach
pub fun unpinned_refusal(a: *A.Allocator, root: str, id: str, version: str) str;
```

## fun dep_outdated

```mach
pub fun dep_outdated(root: str, offline: bool) i64;
```

each version-selected identity's pinned release, the highest this compiler and every range
accept, and the highest release published

## fun add_release

```mach
pub fun add_release(root: str, name: str, url: str, quiet: bool, realize: bool) i64;
```

declare a git dependency at the caret range of the release resolution picks for the running
compiler and realize the closure, as `mach dep add <root> <name> --git <url>` does

root: the project root directory
name: the dependency identity
url: its git source
quiet: suppress progress lines
realize: check the release out; false only resolves the range and writes the table
ret: 0 added, 1 a manifest, resolution or checkout error (printed), 2 the allocator or
         session could not be initialised

