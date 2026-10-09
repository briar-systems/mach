# mach.lang.package.closure

closure: the walk every dependency command shares. it reads the root's declarations,
then the declarations of each realized dependency, whether it was reached by a version
range, a git ref or a path; the root's dep/ holds every identity once, and a version range
any project in the closure declares is pinned there:

- a range's pin is the pin the project records. for a repository root it is the root's own
  gitlink at dep/<id>; for a subproject, a project in a subdirectory of a repository, it is
  the gitlink the enclosing repository commits under the subproject's prefix. pull realizes
  the pin on a fresh clone and restores a checkout that drifted from it, and update records
  the moved pin. a subproject with no pin for a dependency gets a plain clone, as a project
  outside any repository does
- a range a dependency declares resolves with the root's own (mach dep add, mach dep
  update), whether the dependency was reached by a ref or a path: when the root holds no
  checkout of the identity yet, it is seeded by the dependency's own pin for it, update
  --all moves it to the highest release every range admits, and a root declaration of the
  same identity overrides it
- a range with neither a pin nor a checkout under the root is refused with `run mach dep
  update <root> <id>`, which pins it whether the root or a dependency declares it

## val MANIFEST

```mach
pub val MANIFEST: str = manifest.MANIFEST_FILE
```

## rec Operation

```mach
pub rec Operation;
```

one operation on a project's dependencies: the session and storage it works in, the
project root, the tables of the source and pin members, and where it reports

## fun open

```mach
pub fun open(s: *session.Session, a: *A.Allocator, root: str, rep: *report.Report) res[Operation, fail.Fail];
```

open an operation on the project at `root`, released with `close`

## fun close

```mach
pub fun close(op: *Operation);
```

## rec RangeDecl

```mach
pub rec RangeDecl;
```

a further range declared for the identity of a request by another requirer: the closure
keeps one request per identity, and every range on it takes part in resolution

## rec Request

```mach
pub rec Request;
```

one declared dependency of the closure. `via_release` marks a declaration read from a
manifest reached through a release-selected edge, whose requirements the resolver reads
itself from the chosen release; every other declaration is the closure's own. `seed` is the
revision the declaring dependency's own pin holds for a range it declares, "" when it has
none. `decl` is the request's selector as its manifest line spells it; `overrode` holds, on
a root declaration by `ref` or `path`, every differing requirement of the same identity it
replaced; `copied` is what realizing a path dependency did to its copy

## rec Overridden

```mach
pub rec Overridden;
```

a requirement a root declaration replaced: `chain` asked for `asked`

## val COPY_NONE

```mach
pub val COPY_NONE:      u8 = 0
```

## val COPY_REFRESHED

```mach
pub val COPY_REFRESHED: u8 = 1
```

## val COPY_REUSED

```mach
pub val COPY_REUSED:    u8 = 2
```

## rec Walk

```mach
pub rec Walk;
```

how a closure walk reads a fixed git dependency: at the revision its selector picks,
fetching one its checkout lacks unless `offline`. a branch is read at its pin, or at its
fetched tip when the walk is an update that moves it: every one under `all`, or the
identity `name` names

## fun selects_release

```mach
pub fun selects_release(r: *Request) bool;
```

a release-selected git dependency: its requirements are the chosen release's

## fun is_ranged

```mach
pub fun is_ranged(r: *Request) bool;
```

## fun realize

```mach
pub fun realize(op: *Operation, reqs: *Vector[Request], quiet: bool, offline: bool, verify_only: bool,
candidate: *toml.Table, picks: *Vector[resolver.Choice]) err[fail.Fail];
```

walk and realize the closure into `reqs`, the root's manifest read from `candidate` when
it is not nil and otherwise from the project; `verify_only` realizes nothing and refuses a
directory under dep/ outside the closure, where a realizing walk notes and retains it

op: the operation
reqs: receives the closure, which the caller frees
quiet: report no progress lines
offline: fetch nothing
verify_only: change nothing
candidate: the root manifest's table, or nil to read it from the project
picks: the releases resolution chose, or nil
ret: err naming the first failure

## fun pin_command

```mach
pub fun pin_command(a: *A.Allocator, root: str, id: str) res[str, fail.Fail];
```

the command that pins and realizes a version-selected dependency with no checkout yet;
pull's refusal and init's --no-deps hint both name it

## fun unpinned_refusal

```mach
pub fun unpinned_refusal(a: *A.Allocator, root: str, id: str, version: str) res[str, fail.Fail];
```

## fun acquire

```mach
pub fun acquire(op: *Operation, r: *Request, mode: u8, accept: bool, quiet: bool, offline: bool,
pinned_only: bool, effect: *u8) err[fail.Fail];
```

acquire a git dependency's slot through the source and report what changed; `accept` is
add taking an existing checkout, and `pinned_only` a range with no selector, which only a
pin or an existing checkout realizes

## fun index

```mach
pub fun index(reqs: *Vector[Request], id: str) i64;
```

## fun free_requests

```mach
pub fun free_requests(a: *A.Allocator, requests: *Vector[Request]);
```

## fun declared

```mach
pub fun declared(a: *A.Allocator, reqs: *Vector[Request], root: str, mt: *toml.Table) err[fail.Fail];
```

the root's own declarations in the manifest table `mt`, appended to `reqs`

## fun conflict_text

```mach
pub fun conflict_text(a: *A.Allocator, subject: *Request, x: *Request, y: *Request) res[str, fail.Fail];
```

the example table is valid TOML as printed: values are escaped as the
manifest writer would write them

## fun preflight

```mach
pub fun preflight(op: *Operation, doc: *manifest.Doc, reqs: *Vector[Request], inspect_git: bool, walk: *Walk) err[fail.Fail];
```

a fixed git dependency's manifest, and the pins it commits for its ranges, are read at the
revision its selector picks, never from whatever its checkout holds

## fun effects

```mach
pub fun effects(op: *Operation, reqs: *Vector[Request]);
```

a note for each change made before a failure, which the caller shows after it

## fun key_text

```mach
pub fun key_text(t: *toml.Table, key: str) str;
```

a string key as the dependency commands read it: absent reads as empty

## fun parse_file

```mach
pub fun parse_file(a: *A.Allocator, p: str) res[manifest.Doc, fail.Fail];
```

## fun own

```mach
pub fun own(a: *A.Allocator, s: str) str;
```

## fun concat3

```mach
pub fun concat3(a: *A.Allocator, s1: str, s2: str, s3: str) str;
```

## fun oom

```mach
pub fun oom() fail.Fail;
```

