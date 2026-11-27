[mach](README.md) / **session**

# mach.session

the compiler session: the state one process keeps across its builds

a session owns the stores the phases read and write, each worked by its own module: the
diagnostics, the interned names, the types and their facts, the query database, the module
registry, the open buffers, the embedded files and what is retained between builds. it loads a
file's text as an input of the query database, and expires every view borrowed from it when an
input changes.

Source: [`src/session.mach`](../../src/session.mach)

Uses types from [`mach.base.diagnostic`](base/diagnostic.md), [`mach.base.fail`](base/fail.md), [`mach.base.intern`](base/intern.md), [`mach.base.source`](base/source.md), [`mach.base.source.provider.overlay`](base/source/provider/overlay.md), [`mach.session.embed`](session/embed.md), [`mach.session.module`](session/module.md), [`mach.session.query`](session/query.md), [`mach.session.retention`](session/retention.md), [`mach.type`](type.md), [`mach.type.query`](type/query.md).

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
| [`reset_type_projection`](#reset_type_projection) | drop the type store's field projections so they are rebuilt, expiring every borrowed view |
| [`next_parse_incarnation`](#next_parse_incarnation) | the next number naming one parse of a source, counting up from 1 |
| [`load_source`](#load_source) | load a file's text into the source map and set it as the file's input in the query database a text that differs from the one already loaded expires every borrowed view |

| Constants | |
| :--- | :--- |
| [`FILE_TEXT`](#constants) | a file's text as the session loaded it, keyed by the file's index |

---

## Session

```mach
pub rec Session {
    view_generation: u64;
    alloc:           *std.allocator.Allocator;
    build_alloc:     *std.allocator.Allocator;
    # the freeing allocator scratch work draws from and returns to: it outlives
    # the session and is shared by its codegen workers, so it is safe across
    # threads. `alloc` unless the session's owner names its root
    backing: *std.allocator.Allocator;
    # the spans a module's own heap draws from and gives back to, which the session's
    # owner names for a session that compiles; nil for one that never does
    spans: *std.allocator.heap.Pool;
    # the texts every location the session hands out names: its own, or one it shares with
    # the other sessions of an invocation, which outlives them
    sources:      *mach.base.source.SourceMap;
    owns_sources: bool;
    diags:        mach.base.diagnostic.DiagnosticStore;
    # the shared stores, each held behind a handle the session owns, so a worker can be handed a
    # view whose handles name its own layer over them
    interner: *mach.base.intern.Interner;
    types:    *mach.type.TypeInterner;
    # the facts derived from the type store, read and written only by mach.type.query
    type_facts: *mach.type.query.Facts;

    queries:        mach.session.query.QueryDb;
    parse_sequence: u64;

    # the identities of the modules the session has loaded
    modules: mach.session.module.Registry;

    # the open buffers' texts, standing in for their files
    overlays: mach.base.source.provider.overlay.Store;

    embeds: mach.session.embed.EmbedCache;

    # which modules the session keeps products for between builds
    retention: mach.session.retention.Retention;

    # the environment the processes the session starts inherit: the process's own unless the
    # session's owner names another
    environ: std.system.os.Environment;
}
```

the compiler's state across builds: the stores the phases read and write, each owned and
worked by its own module, and the generation of the views borrowed from them

### init

```mach
pub fun init(alloc: *std.allocator.Allocator) res[Session, mach.base.fail.Fail]
```

a session holding its own source map

### init_in

```mach
pub fun init_in(alloc: *std.allocator.Allocator, sources: *mach.base.source.SourceMap) res[Session, mach.base.fail.Fail]
```

a session whose texts go to `sources`, a map that outlives it, so a location it hands out
still names its file once the session is released

### init_over

```mach
pub fun init_over(alloc: *std.allocator.Allocator, base: *mach.base.intern.Interner, sources: *mach.base.source.SourceMap) res[Session, mach.base.fail.Fail]
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
pub fun reset_type_projection(s: *Session) err[mach.base.fail.Fail]
```

drop the type store's field projections so they are rebuilt, expiring every borrowed view

| Parameter | |
| :--- | :--- |
| `s` | the session whose type store is reset |
| **returns** | ok, or a failure when a query operation is active or query products are still borrowed |

### next_parse_incarnation

```mach
pub fun next_parse_incarnation(s: *Session) res[u64, mach.base.fail.Fail]
```

the next number naming one parse of a source, counting up from 1

| Parameter | |
| :--- | :--- |
| `s` | the session that issues the number |
| **returns** | the new number, or a failure once the counter is exhausted |

### load_source

```mach
pub fun load_source(s: *Session, path: str, text: str) res[mach.base.source.FileId, mach.base.fail.Fail]
```

load a file's text into the source map and set it as the file's input in the query database
a text that differs from the one already loaded expires every borrowed view

| Parameter | |
| :--- | :--- |
| `s` | the session to load into |
| `path` | the file's path, cleaned before it keys the source map |
| `text` | the file's text, which the query database copies as the input |
| **returns** | the file's id, or a failure when the path cannot be cleaned, the text is longer than 0xFFFFFFFF bytes, or the source map or query database refuses it |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FILE_TEXT` | `mach.session.query.Kind` | `query.Kind{name: "file text", role: query.INPUT, owner: query.KEY_UNOWNED}` | a file's text as the session loaded it, keyed by the file's index |

