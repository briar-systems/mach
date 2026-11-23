[mach](README.md) / **package**

# mach.package

the dependency manager behind `mach dep` and `mach init`. it reads a project's
dependency closure from its manifests, resolves version ranges against the releases a
source publishes, realizes every dependency flat under dep/ and records its pin, and
checks a realized closure against all of it. a build never reaches it: the driver only
locates what dep/ holds (compile/closure.mach).

the closure engine (closure, resolve, edit, list, verify) works through three contracts,
so a new source is a new member and not a new engine:

- source.mach: where a dependency's releases and content come from
- pin.mach: where a project records the revision each dependency is held at
- report.mach: where progress, notes and failures go while it works

git, under package/git, is the member of the first two and the one outside tool mach
runs; path.mach realizes a dependency declared by a local path, and scratch.mach makes the
temporary storage every part writes, released when the command ends. this module holds what
every part shares: the slot layout under dep/ and the identity rule

Source: [`src/package.mach`](../../src/package.mach)

Uses types from [`mach.base.fail`](base/fail.md), [`mach.manifest.model`](manifest/model.md), [`mach.session`](session.md).

| Functions | |
| :--- | :--- |
| [`name_vector_dnit`](#name_vector_dnit) | free every name in a vector and then the vector itself |
| [`override_note`](#override_note) | the note naming one requirement a root declaration overrode, as `mach dep pull`, `update`, `add` and `verify` print it |
| [`git_source`](#git_source) | the source a manifest's `git` value names, as every git command and record reads it: a relative local path is resolved against the directory of the manifest that declares it, as a `path` is, and made absolute, since git runs in other directories than that one |
| [`ref_of`](#ref_of) | the ref a git line names, empty when it names none |
| [`refuse_nested_realization`](#refuse_nested_realization) | a realized nested dependency (`dep/<id>/dep/<x>/mach.toml`) is refused: the root owns the flat closure and a dependency's own dep/ is never realized |
| [`verify_dep_manifest_id`](#verify_dep_manifest_id) | check that the manifest of a realized dependency declares the id it is realized under |
| [`dep_rel_of`](#dep_rel_of) | the path of a dependency's slot relative to the project root, `dep/<id>` |
| [`dep_full_of`](#dep_full_of) | the path of a dependency's slot under a project root, `<root>/dep/<id>` |
| [`realized_ids`](#realized_ids) | a dependency slot is a directory under dep/ named by a valid project id |

---

## Functions

### name_vector_dnit

```mach
pub fun name_vector_dnit(alloc: *std.allocator.Allocator, names: *Vector[str])
```

free every name in a vector and then the vector itself

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator that owns the names |
| `names` | the vector to release |

### override_note

```mach
pub fun override_note(a: *std.allocator.Allocator, name: str, declared: str, chain: str, requested: str) res[str, mach.base.fail.Fail]
```

the note naming one requirement a root declaration overrode, as `mach dep pull`, `update`,
`add` and `verify` print it

| Parameter | |
| :--- | :--- |
| `a` | allocates the note |
| `name` | the dependency identity |
| `declared` | the root's winning selector, as its manifest line spells it |
| `chain` | the requiring chain, ending at the dependency |
| `requested` | the selector the chain asked for, in the same form |

### git_source

```mach
pub fun git_source(a: *std.allocator.Allocator, manifest_dir: str, spelled: str) res[str, mach.base.fail.Fail]
```

the source a manifest's `git` value names, as every git command and record reads it: a
relative local path is resolved against the directory of the manifest that declares it, as
a `path` is, and made absolute, since git runs in other directories than that one. a url
(`scheme://`, or scp-like `host:path`) and an absolute path are kept as written

| Parameter | |
| :--- | :--- |
| `a` | allocates the source, which the caller owns |
| `manifest_dir` | the directory of the declaring manifest |
| `spelled` | the `git` value as written |
| **returns** | the source, err when the working directory cannot be read |

### ref_of

```mach
pub fun ref_of(s: *mach.session.Session, d: *mach.manifest.model.DepDef) str
```

the ref a git line names, empty when it names none

### refuse_nested_realization

```mach
pub fun refuse_nested_realization(s: *mach.session.Session, alias: str, dep_dir: str) err[mach.base.fail.Fail]
```

a realized nested dependency (`dep/<id>/dep/<x>/mach.toml`) is refused: the
root owns the flat closure and a dependency's own dep/ is never realized.
an empty directory git materializes for a consumed dependency's gitlink is not
a realization and passes

### verify_dep_manifest_id

```mach
pub fun verify_dep_manifest_id(s: *mach.session.Session, dep_full: str, id: str) err[mach.base.fail.Fail]
```

check that the manifest of a realized dependency declares the id it is realized under

| Parameter | |
| :--- | :--- |
| `s` | the session whose allocator, interner and source map read the manifest |
| `dep_full` | the dependency's root directory |
| `id` | the identity it is realized under, its directory name under dep/ |
| **returns** | ok when the declared id matches, err when the manifest cannot be read, declares no readable id or declares a different one |

### dep_rel_of

```mach
pub fun dep_rel_of(alloc: *std.allocator.Allocator, id: str) res[str, mach.base.fail.Fail]
```

the path of a dependency's slot relative to the project root, `dep/<id>`

| Parameter | |
| :--- | :--- |
| `alloc` | allocates the path, which the caller owns |
| `id` | the dependency identity |
| **returns** | the relative path, err when it cannot be formatted |

### dep_full_of

```mach
pub fun dep_full_of(alloc: *std.allocator.Allocator, root: str, id: str) res[str, mach.base.fail.Fail]
```

the path of a dependency's slot under a project root, `<root>/dep/<id>`

| Parameter | |
| :--- | :--- |
| `alloc` | allocates the path, which the caller owns |
| `root` | the project root |
| `id` | the dependency identity |
| **returns** | the joined path, err when it cannot be built |

### realized_ids

```mach
pub fun realized_ids(alloc: *std.allocator.Allocator, root: str) res[Vector[str], mach.base.fail.Fail]
```

a dependency slot is a directory under dep/ named by a valid project id

