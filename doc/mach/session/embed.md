[mach](../README.md) / [session](README.md) / **embed**

# mach.session.embed

the files a build embeds, read and digested once and watched as query inputs

an `embed` argument resolves against its declaring file, or as a manifest template against the
root project in its project's scope, and a resolved path is checked against escaping the root.
a refresh reads the file, keeps its bytes and sha-256 digest, and publishes its status and
digest as an input of the query database, so a module that embeds it recomputes only when it
changed.

Source: [`src/session/embed.mach`](../../../src/session/embed.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.handle`](../base/handle.md), [`mach.base.intern`](../base/intern.md), [`mach.manifest.template`](../manifest/template.md), [`mach.session.query`](query.md).

| Type | | |
| :--- | :--- | :--- |
| [`EmbedFile`](#embedfile) | `rec` | one embedded file as the cache last read it |
| [`EmbedCache`](#embedcache) | `rec` | the embedded files of one build, found by interned path |
| [`Resolution`](#resolution) | `rec` | what an `embed` argument resolves to |

| On `EmbedCache` | |
| :--- | :--- |
| [`init`](#init) | an empty cache with no root and no scopes bound, building artifacts |
| [`set_path_env`](#set_path_env) | bind the build a path resolves in: the root embeds are contained in and templates expand against, and the requirement scope of each project whose modules the build compiles |
| [`dnit`](#dnit) | release every entry's bytes and the entry store |
| [`get`](#get) | the entry for a path the cache holds |
| [`resolve_arg`](#resolve_arg) | resolve an `embed` argument: a literal path against the declaring file's directory, a template against the root in the declaring module's project scope |
| [`escapes_root`](#escapes_root) | check lexical and existing-parent containment beneath the project root a missing parent is left to embed loading, while an incomplete check is err |
| [`refresh`](#refresh) | read a file into its entry and publish its status and digest as a query input a file that is missing, a directory, unreadable or too large is recorded in the entry's status, not returned as a failure |
| [`count`](#count) | the number of entries the cache holds |
| [`insert`](#insert) | add an entry under a path the cache does not hold yet, into a chunk it never leaves; the entry's bytes belong to the cache from here and dnit frees them |

| Constants | |
| :--- | :--- |
| [`EMBED_OK`](#constants), [`EMBED_UNREADABLE`](#constants) | |
| [`EMBED_FILE`](#constants) | a file a module embeds, as the cache last saw it: its status and digest, keyed by the interned path |

---

## EmbedFile

```mach
pub rec EmbedFile {
    path:   mach.base.intern.StrId;
    status: EmbedStatus;
    reason: str;
    bytes:  *u8;
    len:    u32;
    digest: [32]u8;
}
```

one embedded file as the cache last read it

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `mach.base.intern.StrId` | the file's interned path |
| `status` | `EmbedStatus` | whether it was read, or why not (missing, a directory, unreadable, too large) |
| `reason` | `str` | the text of a failed status, empty when the read succeeded |
| `bytes` | `*u8` | the file's contents, owned by the cache, nil when it is empty or was not read |
| `len` | `u32` | the byte length of `bytes` |
| `digest` | `[32]u8` | the sha-256 of the contents, all zero when the file was not read |

## EmbedCache

```mach
pub rec EmbedCache {
    alloc:   *std.allocator.Allocator;
    entries: mach.base.handle.StableChunks[EmbedFile];

    root:        str;
    scopes:      *mach.manifest.template.Requirements;
    scope_count: u32;

    # whether this invocation produces the artifact outputs its templates name.
    # a check does not, so a missing one is a fact about the command rather than
    # about the source, and the diagnostic has to say which
    builds_artifacts: bool;
}
```

the embedded files of one build, found by interned path

the store is a handle.StableChunks: an entry is written once into a fixed
chunk and never moves, so a `*EmbedFile` from `get` or `insert` stays valid
until `dnit` however many files a later refresh adds. a refresh rewrites the
entry in place, so a holder reads the current status, bytes and digest

### init

```mach
pub fun init(alloc: *std.allocator.Allocator) EmbedCache
```

an empty cache with no root and no scopes bound, building artifacts

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator the cache's entries and their bytes come from |
| **returns** | the cache, released with `dnit` |

### set_path_env

```mach
pub fun set_path_env(c: *EmbedCache, root: str, scopes: *mach.manifest.template.Requirements, scope_count: u32,
    builds: bool)
```

bind the build a path resolves in: the root embeds are contained in and
templates expand against, and the requirement scope of each project whose
modules the build compiles

| Parameter | |
| :--- | :--- |
| `c` | the cache |
| `root` | the root project's directory |
| `scopes` | one scope per project; borrowed for as long as the build runs |
| `scope_count` | length of `scopes` |
| `builds` | whether the build writes the artifact outputs templates name |

### dnit

```mach
pub fun dnit(c: *EmbedCache)
```

release every entry's bytes and the entry store

| Parameter | |
| :--- | :--- |
| `c` | the cache to release |

### get

```mach
pub fun get(c: *EmbedCache, path_id: mach.base.intern.StrId) opt[*EmbedFile]
```

the entry for a path the cache holds

the returned `*EmbedFile` is stable until `dnit`: the store never moves an
entry when more files are added, so the pointer may be held across a refresh
that reads other files, and reads the entry's current state

### resolve_arg

```mach
pub fun resolve_arg(alloc: *std.allocator.Allocator, c: *EmbedCache, itn: *mach.base.intern.Interner, module_fqn: mach.base.intern.StrId,
    decl_file: str, arg: str) res[Resolution, mach.base.fail.Fail]
```

resolve an `embed` argument: a literal path against the declaring file's
directory, a template against the root in the declaring module's project scope

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned path and a rejection's text |
| `c` | the cache holding the path environment |
| `itn` | resolves the module and scope names |
| `module_fqn` | the declaring module, whose head segment names its project |
| `decl_file` | the declaring file's path |
| `arg` | the decorator's path argument |
| **returns** | the resolution; err as `template.expand` refuses the template in an `embed` path, or when the build has no project root to resolve it in |

### escapes_root

```mach
pub fun escapes_root(alloc: *std.allocator.Allocator, c: *EmbedCache, resolved: str) res[bool, mach.base.fail.Fail]
```

check lexical and existing-parent containment beneath the project root
a missing parent is left to embed loading, while an incomplete check is err

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator the check's scratch paths come from |
| `c` | the cache holding the project root |
| `resolved` | the resolved `embed` path |
| **returns** | true when the path lies outside the root or a component of its existing parent is a symlink, false when no root is bound, the root is missing or the path is contained. err when the working directory cannot be read, the root or a component cannot be inspected, the root is not a directory, the path crosses a non-directory, or an allocation is refused |

### refresh

```mach
pub fun refresh(c: *EmbedCache, db: *mach.session.query.QueryDb, itn: *mach.base.intern.Interner, p: str) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

read a file into its entry and publish its status and digest as a query input
a file that is missing, a directory, unreadable or too large is recorded in the entry's
status, not returned as a failure

| Parameter | |
| :--- | :--- |
| `c` | the cache holding the entry, created here when the path is new |
| `db` | the query database the entry is published to |
| `itn` | interns the path |
| `p` | the file's resolved path |
| **returns** | the interned path, or a failure when interning, storing, copying the bytes or setting the input is refused |

### count

```mach
pub fun count(c: *EmbedCache) u32
```

the number of entries the cache holds

### insert

```mach
pub fun insert(c: *EmbedCache, e: EmbedFile) res[*EmbedFile, mach.base.fail.Fail]
```

add an entry under a path the cache does not hold yet, into a chunk it never
leaves; the entry's bytes belong to the cache from here and dnit frees them

## Resolution

```mach
pub rec Resolution {
    path:     str;
    artifact: mach.base.intern.StrId;
}
```

what an `embed` argument resolves to

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `str` | the file it names, owned by the allocator resolve_arg was given |
| `artifact` | `mach.base.intern.StrId` | the artifact whose output a template names, nil for a literal path or a template that names none |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `EMBED_OK` | `EmbedStatus` | `0` |  |
| `EMBED_UNREADABLE` | `EmbedStatus` | `3` |  |
| `EMBED_FILE` | `mach.session.query.Kind` | `query.Kind{name: "embedded file", role: query.INPUT, owner: query.KEY_UNOWNED}` | a file a module embeds, as the cache last saw it: its status and digest, keyed by the interned path |

