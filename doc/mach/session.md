[mach](README.md) / **session**

# mach.session

the compiler session: the state one process keeps across its builds

a session owns the stores the phases read and write, each worked by its own module: the
diagnostics, the interned names, the types and their facts, the query database, the module
registry, the open buffers, the embedded files and what is retained between builds. it loads a
file's text as an input of the query database, and expires every view borrowed from it when an
input changes.

<sub>source: [`src/session.mach`](../../src/session.mach) · imported by 53 modules</sub>

<sub>`A` is `std.allocator` · `heap` is `std.allocator.heap` · `base_source` is [`mach.base.source`](base/source.md) · `diagnostic` is [`mach.base.diagnostic`](base/diagnostic.md) · `intern` is [`mach.base.intern`](base/intern.md) · `type` is [`mach.type`](type.md) · `type_query` is [`mach.type.query`](type/query.md) · `query` is [`mach.session.query`](session/query.md) · `module` is [`mach.session.module`](session/module.md) · `overlay` is [`mach.base.source.provider.overlay`](base/source/provider/overlay.md) · `session_embed` is [`mach.session.embed`](session/embed.md) · `retention` is [`mach.session.retention`](session/retention.md) · `exec` is `std.process.exec` · `fail` is [`mach.base.fail`](base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Session`](#session) | `rec` | the compiler's state across builds: the stores the phases read and write, each owned and worked by its own module, and the generation of the views borrowed from them |

| On `Session` | |
| :--- | :--- |
| [`init`](#init) | a session holding its own source map |
| [`init_in`](#init_in) | a session whose texts go to `sources`, a map that outlives it, so a location it hands out still names its file once the session is released |
| [`init_over`](#init_over) | a session whose names are layered over `base` and whose texts go to `sources`: every id `base` has issued names the same string here and every file of `sources` the same text, so what was planned in another session, its manifests and its cells, is read and located here as it was planned |
| [`dnit`](#dnit) | release a session and everything it owns; a nil session holds nothing |
| [`expire_views`](#expire_views) | a change to the session's inputs or module identities expires every view borrowed from it |
| [`reset_type_projection`](#reset_type_projection) · [`next_parse_incarnation`](#next_parse_incarnation) · [`load_source`](#load_source) | |

| Constants | |
| :--- | :--- |
| [`FILE_TEXT`](#constants) | a file's text as the session loaded it, keyed by the file's index |

---

## Session

```mach
pub rec Session {
    view_generation: u64;
    alloc:           *A.Allocator;
    build_alloc:     *A.Allocator;
    # the freeing allocator scratch work draws from and returns to: it outlives
    # the session and is shared by its codegen workers, so it is safe across
    # threads. `alloc` unless the session's owner names its root
    backing: *A.Allocator;
    # the spans a module's own heap draws from and gives back to, which the session's
    # owner names for a session that compiles; nil for one that never does
    spans: *heap.Pool;
    # the texts every location the session hands out names: its own, or one it shares with
    # the other sessions of an invocation, which outlives them
    sources:      *base_source.SourceMap;
    owns_sources: bool;
    diags:        diagnostic.DiagnosticStore;
    interner:     intern.Interner;
    types:        type.TypeInterner;
    # the facts derived from the type store, read and written only by mach.type.query
    type_facts: type_query.Facts;

    queries:        query.QueryDb;
    parse_sequence: u64;

    # the identities of the modules the session has loaded
    modules: module.Registry;

    # the open buffers' texts, standing in for their files
    overlays: overlay.Store;

    embeds: session_embed.EmbedCache;

    # which modules the session keeps products for between builds
    retention: retention.Retention;

    # the environment the processes the session starts inherit: the process's own unless the
    # session's owner names another
    environ: exec.Environment;
}
```

the compiler's state across builds: the stores the phases read and write, each owned and
worked by its own module, and the generation of the views borrowed from them

### init

```mach
pub fun init(alloc: *A.Allocator) res[Session, fail.Fail]
```

a session holding its own source map

### init_in

```mach
pub fun init_in(alloc: *A.Allocator, sources: *base_source.SourceMap) res[Session, fail.Fail]
```

a session whose texts go to `sources`, a map that outlives it, so a location it hands out
still names its file once the session is released

### init_over

```mach
pub fun init_over(alloc: *A.Allocator, base: *intern.Interner, sources: *base_source.SourceMap) res[Session, fail.Fail]
```

a session whose names are layered over `base` and whose texts go to `sources`: every id
`base` has issued names the same string here and every file of `sources` the same text, so
what was planned in another session, its manifests and its cells, is read and located here as
it was planned. `base` and `sources` outlive the session, and `base` takes no new names while
it lives

### dnit

```mach
pub fun dnit(s: *Session)
```

release a session and everything it owns; a nil session holds nothing

### expire_views

```mach
pub fun expire_views(s: *Session)
```

a change to the session's inputs or module identities expires every view borrowed from it. exhaustion
permanently refuses new borrowed views while teardown remains infallible

### reset_type_projection

```mach
pub fun reset_type_projection(s: *Session) err[fail.Fail]
```

### next_parse_incarnation

```mach
pub fun next_parse_incarnation(s: *Session) res[u64, fail.Fail]
```

### load_source

```mach
pub fun load_source(s: *Session, path: str, text: str) res[base_source.FileId, fail.Fail]
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FILE_TEXT` | `query.Kind` | `query.Kind{name: "file text", role: query.INPUT, owner: query.KEY_UNOWNED}` | a file's text as the session loaded it, keyed by the file's index |

