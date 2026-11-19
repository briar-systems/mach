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
| [`name_vector_dnit`](#name_vector_dnit) | |
| [`override_note`](#override_note) | the note naming one requirement a root declaration overrode, as `mach dep pull`, `update`, `add` and `verify` print it |
| [`ref_of`](#ref_of) | the ref a git line names, empty when it names none |
| [`check_dep_identity`](#check_dep_identity) | the manifest key, the directory under dep/ and the project id are one name |
| [`refuse_nested_realization`](#refuse_nested_realization) | a realized nested dependency (`dep/<id>/dep/<x>/mach.toml`) is refused: the root owns the flat closure and a dependency's own dep/ is never realized |
| [`verify_dep_manifest_id`](#verify_dep_manifest_id), [`dep_rel_of`](#dep_rel_of), [`dep_full_of`](#dep_full_of) | |
| [`realized_ids`](#realized_ids) | a dependency slot is a directory under dep/ named by a valid project id |

---

## Functions

### name_vector_dnit

```mach
pub fun name_vector_dnit(alloc: *std.allocator.Allocator, names: *Vector[str])
```

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

### ref_of

```mach
pub fun ref_of(s: *mach.session.Session, d: *mach.manifest.model.DepDef) str
```

the ref a git line names, empty when it names none

### check_dep_identity

```mach
pub fun check_dep_identity(s: *mach.session.Session, key: str, declared: str) err[mach.base.fail.Fail]
```

the manifest key, the directory under dep/ and the project id are one name

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

### dep_rel_of

```mach
pub fun dep_rel_of(alloc: *std.allocator.Allocator, id: str) res[str, mach.base.fail.Fail]
```

### dep_full_of

```mach
pub fun dep_full_of(alloc: *std.allocator.Allocator, root: str, id: str) res[str, mach.base.fail.Fail]
```

### realized_ids

```mach
pub fun realized_ids(alloc: *std.allocator.Allocator, root: str) res[Vector[str], mach.base.fail.Fail]
```

a dependency slot is a directory under dep/ named by a valid project id

