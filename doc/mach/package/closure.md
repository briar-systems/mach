[mach](../README.md) / [package](README.md) / **closure**

# mach.package.closure

closure: the walk every dependency command shares. it reads the root's declarations,
then the declarations of each realized dependency, whether it was reached by a version
range, a git ref or a path; the root's dep/ holds every identity once, and a version range
any project in the closure declares is pinned there:

- a range's pin is the pin the project records. for a repository root it is the root's own
  gitlink at dep/\<id>; for a subproject, a project in a subdirectory of a repository, it is
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

Source: [`src/package/closure.mach`](../../../src/package/closure.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.manifest.load`](../manifest/load.md), [`mach.package.pin`](pin.md), [`mach.package.report`](report.md), [`mach.package.resolver`](resolver.md), [`mach.package.source`](source.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Operation`](#operation) | `rec` | one operation on a project's dependencies: the session and storage it works in, the project root, the tables of the source and pin members, and where it reports |
| [`Request`](#request) | `rec` | one declared dependency of the closure |
| [`Overridden`](#overridden) | `rec` | a requirement a root declaration replaced: `chain` asked for `asked` |
| [`Walk`](#walk) | `rec` | how a closure walk reads a fixed git dependency: at the revision its selector picks, fetching one its checkout lacks unless `offline` |

| On `Operation` | |
| :--- | :--- |
| [`init`](#init) | open an operation on the project at `root`, which need not exist yet, released with `dnit` |
| [`dnit`](#dnit) | release the operation; err names the releases table's scratch it could not remove, once the rest is released |
| [`ended`](#ended) | `outcome` of the work an operation held, then the operation released: a failure to release it is the outcome when the work succeeded, and is reported beside it otherwise |
| [`releases`](#releases) | the releases table of the operation, opened by its first resolution as `walk` reads: offline or not, and announcing each url it reads from the network unless quiet |
| [`realize`](#realize) | walk and realize the closure into `reqs`, the root's manifest read from `candidate` when it is not nil and otherwise from the project; `verify_only` realizes nothing and refuses a directory under dep/ outside the closure, where a realizing walk notes and retains it |
| [`sync_path`](#sync_path) | sync the copy of the path dependency `id` with its source `src`, saying what the sync did unless quiet |
| [`acquire`](#acquire) | acquire a git dependency's slot through the source and report what changed; `accept` is add taking an existing checkout, and `pinned_only` a range with no selector, which only a pin or an existing checkout realizes |
| [`preflight`](#preflight) | a fixed git dependency's manifest, and the pins it commits for its ranges, are read at the revision its selector picks, never from whatever its checkout holds |
| [`effects`](#effects) | a note for each change made before a failure, which the caller shows after it |

| On `Request` | |
| :--- | :--- |
| [`selects_release`](#selects_release) | a release-selected git dependency: its requirements are the chosen release's |
| [`index`](#index) | the position of an identity among the requests |
| [`requests_dnit`](#requests_dnit) | free every request and then the vector itself |
| [`declared`](#declared) | the root's own declarations in the manifest table `mt`, appended to `reqs` |
| [`conflict_text`](#conflict_text) | the example table is valid TOML as printed: values are escaped as the manifest writer would write them |

| Functions | |
| :--- | :--- |
| [`pin_command`](#pin_command) | the command that pins and realizes a version-selected dependency with no checkout yet; pull's refusal and init's --no-deps hint both name it |
| [`unpinned_refusal`](#unpinned_refusal) | the refusal text for a version-selected dependency with no pin and no checkout yet |
| [`key_text`](#key_text) | a string key as the dependency commands read it: absent reads as empty |
| [`parse_file`](#parse_file) | read and parse a manifest file, naming the file when it is not valid TOML |

| Constants | |
| :--- | :--- |
| [`COPY_NONE`](#constants) | |

---

## Operation

```mach
pub rec Operation {
    s:             *mach.session.Session;
    a:             *std.allocator.Allocator;
    root:          str;
    checkouts:     mach.package.source.Checkouts;
    pins:          mach.package.pin.Pin;
    rep:           *mach.package.report.Report;
    releases:      mach.package.source.Releases;
    releases_open: bool;
    synced:        str;
}
```

one operation on a project's dependencies: the session and storage it works in, the
project root, the tables of the source and pin members, and where it reports. `releases`
is the one table every resolution of the operation reads a url's releases from, so a url is
listed and its manifests fetched once however many times the operation resolves; it is
opened by the first resolution, and `releases_open` says whether it has been. `synced` is
the id of the path dependency whose copy the operation has already synced, nil before one

### init

```mach
pub fun init(s: *mach.session.Session, a: *std.allocator.Allocator, root: str, rep: *mach.package.report.Report) res[Operation, mach.base.fail.Fail]
```

open an operation on the project at `root`, which need not exist yet, released with `dnit`

### dnit

```mach
pub fun dnit(op: *Operation) err[mach.base.fail.Fail]
```

release the operation; err names the releases table's scratch it could not remove, once the
rest is released

### ended

```mach
pub fun ended(op: *Operation, outcome: err[mach.base.fail.Fail]) err[mach.base.fail.Fail]
```

`outcome` of the work an operation held, then the operation released: a failure to release
it is the outcome when the work succeeded, and is reported beside it otherwise

### releases

```mach
pub fun releases(op: *Operation, walk: *Walk) res[*mach.package.source.Releases, mach.base.fail.Fail]
```

the releases table of the operation, opened by its first resolution as `walk` reads: offline
or not, and announcing each url it reads from the network unless quiet. every later
resolution of the operation reads the same table

### realize

```mach
pub fun realize(op: *Operation, reqs: *Vector[Request], quiet: bool, unchanged: bool, offline: bool, verify_only: bool,
    candidate: *toml.value.Table, picks: *Vector[mach.package.resolver.Choice]) err[mach.base.fail.Fail]
```

walk and realize the closure into `reqs`, the root's manifest read from `candidate` when
it is not nil and otherwise from the project; `verify_only` realizes nothing and refuses a
directory under dep/ outside the closure, where a realizing walk notes and retains it

| Parameter | |
| :--- | :--- |
| `op` | the operation |
| `reqs` | receives the closure, which the caller frees |
| `quiet` | report no results |
| `unchanged` | report a git dependency found already realized as unchanged, as pull does |
| `offline` | fetch nothing |
| `verify_only` | change nothing |
| `candidate` | the root manifest's table, or nil to read it from the project |
| `picks` | the releases resolution chose, or nil |
| **returns** | err naming the first failure |

### sync_path

```mach
pub fun sync_path(op: *Operation, id: str, src: str, quiet: bool) res[u8, mach.base.fail.Fail]
```

sync the copy of the path dependency `id` with its source `src`, saying what the sync did
unless quiet. the operation records the sync, so its closure walk leaves the copy as it is

| Parameter | |
| :--- | :--- |
| **returns** | the copy outcome, one of package_path's COPY_MADE, COPY_REFRESHED and COPY_REUSED |

### acquire

```mach
pub fun acquire(op: *Operation, r: *Request, mode: u8, accept: bool, quiet: bool, offline: bool,
    pinned_only: bool, effect: *u8) err[mach.base.fail.Fail]
```

acquire a git dependency's slot through the source and report what changed; `accept` is
add taking an existing checkout, and `pinned_only` a range with no selector, which only a
pin or an existing checkout realizes

### preflight

```mach
pub fun preflight(op: *Operation, doc: *mach.manifest.load.Doc, reqs: *Vector[Request], inspect_git: bool, walk: *Walk) err[mach.base.fail.Fail]
```

a fixed git dependency's manifest, and the pins it commits for its ranges, are read at the
revision its selector picks, never from whatever its checkout holds

### effects

```mach
pub fun effects(op: *Operation, reqs: *Vector[Request])
```

a note for each change made before a failure, which the caller shows after it

## Request

```mach
pub rec Request {
    id:          str;
    url:         str;
    src_path:    str;
    ref:         str;
    version:     str;
    chain:       str;
    requirer:    str;
    root_decl:   bool;
    is_git:      bool;
    via_release: bool;
    seed:        str;
    effect:      str;
    also:        Vector[RangeDecl];
    decl:        str;
    overrode:    Vector[Overridden];
    copied:      u8;
}
```

one declared dependency of the closure. `via_release` marks a declaration read from a
manifest reached through a release-selected edge, whose requirements the resolver reads
itself from the chosen release; every other declaration is the closure's own. `seed` is the
revision the declaring dependency's own pin holds for a range it declares, nil when it has
none. `decl` is the request's selector as its manifest line spells it; `overrode` holds, on
a root declaration by `ref` or `path`, every differing requirement of the same identity it
replaced; `copied` is what realizing a path dependency did to its copy, COPY_NONE or one of
package_path's copy outcomes

### selects_release

```mach
pub fun selects_release(r: *Request) bool
```

a release-selected git dependency: its requirements are the chosen release's

### index

```mach
pub fun index(reqs: *Vector[Request], id: str) i64
```

the position of an identity among the requests

| Parameter | |
| :--- | :--- |
| `reqs` | the closure |
| `id` | the identity to find |
| **returns** | its index, -1 when no request has it |

### requests_dnit

```mach
pub fun requests_dnit(a: *std.allocator.Allocator, requests: *Vector[Request])
```

free every request and then the vector itself

| Parameter | |
| :--- | :--- |
| `a` | the allocator that owns the requests' strings |
| `requests` | the vector to release |

### declared

```mach
pub fun declared(a: *std.allocator.Allocator, reqs: *Vector[Request], root: str, mt: *toml.value.Table) err[mach.base.fail.Fail]
```

the root's own declarations in the manifest table `mt`, appended to `reqs`

### conflict_text

```mach
pub fun conflict_text(a: *std.allocator.Allocator, subject: *Request, x: *Request, y: *Request) res[str, mach.base.fail.Fail]
```

the example table is valid TOML as printed: values are escaped as the
manifest writer would write them

## Overridden

```mach
pub rec Overridden {
    chain: str;
    asked: str;
}
```

a requirement a root declaration replaced: `chain` asked for `asked`

## Walk

```mach
pub rec Walk {
    offline: bool;
    all:     bool;
    name:    str;
    quiet:   bool;
}
```

how a closure walk reads a fixed git dependency: at the revision its selector picks,
fetching one its checkout lacks unless `offline`. a branch is read at its pin, or at its
fetched tip when the walk is an update that moves it: every one under `all`, or the
identity `name` names. a walk that reads releases from the network says so first unless `quiet`

## Functions

### pin_command

```mach
pub fun pin_command(a: *std.allocator.Allocator, root: str, id: str) res[str, mach.base.fail.Fail]
```

the command that pins and realizes a version-selected dependency with no checkout yet;
pull's refusal and init's --no-deps hint both name it

### unpinned_refusal

```mach
pub fun unpinned_refusal(a: *std.allocator.Allocator, root: str, id: str, version: str) res[str, mach.base.fail.Fail]
```

the refusal text for a version-selected dependency with no pin and no checkout yet

| Parameter | |
| :--- | :--- |
| `a` | allocates the text, which the caller owns |
| `root` | the project root, as the command names it |
| `id` | the dependency identity |
| `version` | the range it declares |
| **returns** | the text naming the command that pins it, err when it cannot be formatted |

### key_text

```mach
pub fun key_text(t: *toml.value.Table, key: str) str
```

a string key as the dependency commands read it: absent reads as empty

### parse_file

```mach
pub fun parse_file(a: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, p: str) res[mach.manifest.load.Doc, mach.base.fail.Fail]
```

read and parse a manifest file, naming the file when it is not valid TOML

| Parameter | |
| :--- | :--- |
| `a` | allocates the document, which the caller frees with doc_dnit |
| `sources` | the source map the file is registered in |
| `p` | the manifest path |
| **returns** | the document, err when it cannot be read or parsed |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COPY_NONE` | `u8` | `0` |  |

