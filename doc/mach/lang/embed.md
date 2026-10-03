# mach.lang.embed

## val EMBED_OK

```mach
pub val EMBED_OK:         EmbedStatus = 0
```

## val EMBED_UNREADABLE

```mach
pub val EMBED_UNREADABLE: EmbedStatus = 3
```

## rec EmbedFile

```mach
pub rec EmbedFile;
```

## rec EmbedCache

```mach
pub rec EmbedCache;
```

the embedded files of one build, found by interned path

the store is a handle.StableChunks: an entry is written once into a fixed
chunk and never moves, so a `*EmbedFile` from `get` or `insert` stays valid
until `dnit` however many files a later refresh adds. a refresh rewrites the
entry in place, so a holder reads the current status, bytes and digest

## fun init

```mach
pub fun init(alloc: *A.Allocator) EmbedCache;
```

## fun set_path_env

```mach
pub fun set_path_env(c: *EmbedCache, root: str, scopes: *manifest.RequirementScope, scope_count: u32,
builds: bool);
```

bind the build a path resolves in: the root embeds are contained in and
templates expand against, and the requirement scope of each project whose
modules the build compiles

c: the cache
root: the root project's directory
scopes: one scope per project; borrowed for as long as the build runs
scope_count: length of `scopes`
builds: whether the build writes the artifact outputs templates name

## fun dnit

```mach
pub fun dnit(c: *EmbedCache);
```

## fun get

```mach
pub fun get(c: *EmbedCache, path_id: intern.StrId) opt[*EmbedFile];
```

the entry for a path the cache holds

the returned `*EmbedFile` is stable until `dnit`: the store never moves an
entry when more files are added, so the pointer may be held across a refresh
that reads other files, and reads the entry's current state

## fun resolve_arg

```mach
pub fun resolve_arg(alloc: *A.Allocator, c: *EmbedCache, itn: *intern.Interner, module_fqn: intern.StrId,
decl_file: str, arg: str) res[str, manifest.TemplateError];
```

resolve an `embed` argument: a literal path against the declaring file's
directory, a template against the root in the declaring module's project scope

alloc: owns the returned path and a rejection's text
c: the cache holding the path environment
itn: resolves the module and scope names
module_fqn: the declaring module, whose head segment names its project
decl_file: the declaring file's path
arg: the decorator's path argument
ret: the resolved path; err as `manifest.expand_artifact_path`

## fun escapes_root

```mach
pub fun escapes_root(alloc: *A.Allocator, c: *EmbedCache, resolved: str) res[bool, fail.Fail];
```

## fun refresh

```mach
pub fun refresh(c: *EmbedCache, db: *query.QueryDb, itn: *intern.Interner, p: str) res[intern.StrId, fail.Fail];
```

## fun count

```mach
pub fun count(c: *EmbedCache) u32;
```

the number of entries the cache holds

## fun insert

```mach
pub fun insert(c: *EmbedCache, e: EmbedFile) res[*EmbedFile, fail.Fail];
```

add an entry under a path the cache does not hold yet, into a chunk it never
leaves; the entry's bytes belong to the cache from here and dnit frees them

