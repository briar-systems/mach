# mach.lang.package

package: the dependency manager behind `mach dep` and `mach init`. it reads a project's
dependency closure from its manifests, resolves version ranges against the releases a
source publishes, realizes every dependency flat under dep/ and records its pin, and
checks a realized closure against all of it. a build never reaches it: the driver only
locates what dep/ holds (driver/closure.mach).

the closure engine (closure, resolve, edit, list, verify) works through three contracts,
so a new source is a new member and not a new engine:

- source.mach: where a dependency's releases and content come from
- pin.mach: where a project records the revision each dependency is held at
- report.mach: where progress, notes and failures go while it works

git, under package/git, is the member of the first two and the one outside tool mach
runs; path.mach realizes a dependency declared by a local path. this module holds what
every part shares: the slot layout under dep/ and the identity rule

## fun free_name_vector

```mach
pub fun free_name_vector(alloc: *A.Allocator, names: *Vector[str]);
```

## fun override_note

```mach
pub fun override_note(a: *A.Allocator, name: str, declared: str, chain: str, requested: str) res[str, fail.Fail];
```

the note naming one requirement a root declaration overrode, as `mach dep pull`, `update`,
`add` and `verify` print it

a: allocates the note
name: the dependency identity
declared: the root's winning selector, as its manifest line spells it
chain: the requiring chain, ending at the dependency
requested: the selector the chain asked for, in the same form

## fun intern_text

```mach
pub fun intern_text(s: *session.Session, id: intern.StrId) str;
```

## fun check_dep_identity

```mach
pub fun check_dep_identity(s: *session.Session, key: str, declared: str) err[fail.Fail];
```

the manifest key, the directory under dep/ and the project id are one name

## fun refuse_nested_realization

```mach
pub fun refuse_nested_realization(s: *session.Session, alias: str, dep_dir: str) err[fail.Fail];
```

a realized nested dependency (`dep/<id>/dep/<x>/mach.toml`) is refused: the
root owns the flat closure and a dependency's own dep/ is never realized.
an empty directory git materializes for a consumed dependency's gitlink is not
a realization and passes

## fun verify_dep_manifest_id

```mach
pub fun verify_dep_manifest_id(s: *session.Session, dep_full: str, id: str) err[fail.Fail];
```

## fun dep_rel_of

```mach
pub fun dep_rel_of(alloc: *A.Allocator, id: str) res[str, fail.Fail];
```

## fun dep_full_of

```mach
pub fun dep_full_of(alloc: *A.Allocator, root: str, id: str) res[str, fail.Fail];
```

## fun realized_ids

```mach
pub fun realized_ids(alloc: *A.Allocator, root: str) res[Vector[str], fail.Fail];
```

a dependency slot is a directory under dep/ named by a valid project id

