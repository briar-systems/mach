[mach](../README.md) / [compile](README.md) / **readahead**

# mach.compile.readahead

files read and parsed ahead of the load walk. the walk stays on the owner thread and follows `use`
lines in order; each module it parses proposes the files its `use` and `fwd` paths may name, and a
worker reads, lexes and parses each into a tree and a name table of its own. when the walk reaches a
file it takes what was read ahead for that path, or reads and parses it as it would alone. a
finished job proposes in turn the files its own tree names, so the frontier runs ahead of the walk
as far as the threads keep up, not one module deep. the owner
merges a taken parse where a single thread would have made it, its names in the order the parse
interned them and its diagnostics in the order they were reported, so every id is the one a single
thread issues. a proposal costs no id: it is spelled out and located by text, and a file the walk
never reaches is dropped unread or unmerged

a worker reads only the file system and the overlays (provider `peek`) and writes only its job.
each job allocates on a heap of its own over the session's span pool, which takes no lock, so the
threads do not queue on the session's heap. a merge copies the tree onto the parse query's
allocator and the job's heap goes whole

Source: [`src/compile/readahead.mach`](../../../src/compile/readahead.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source.provider`](../base/source/provider.md), [`mach.compile.workers`](workers.md), [`mach.front.ast`](../front/ast.md), [`mach.front.parser.state`](../front/parser/state.md), [`mach.report.phase`](../report/phase.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Taken`](#taken) | `rec` | a parse the walk took, which the parse query merges |
| [`ReadAhead`](#readahead) | `rec` | a:       owns the jobs and their paths; spans: what each job's heap draws from sources: the walk's sources, which locate on the owner and read on any thread crew:    the threads; slots: free task indices, each in-flight job holding one (slot_job) pending: proposed jobs not yet handed to a thread, newest last and handed first: the walk goes          depth first, so it takes the first file a module names itself while the threads read          on past it |

| On `ReadAhead` | |
| :--- | :--- |
| [`open`](#open) | a read-ahead on `threads` threads for session `s` over the walk's `sources`, or false when it cannot be had (one thread, a session with no span pool, storage, or a thread the host refused), holding nothing then |
| [`close`](#close) | stop the threads, dropping what is still queued, and give back every job the walk left |
| [`propose_uses`](#propose_uses) | the files a parsed module's `use` and `fwd` paths may name proposed: each path and each of its prefixes of two segments or more, as the load resolves a path to its longest prefix that is a module |
| [`text_for`](#text_for) | the text the walk reads at `path`: what was read ahead, waiting for it if a thread is on it, or none for the walk to read itself |
| [`parse_for`](#parse_for) | the parse read ahead for the file at `path`, once the walk took its text, for the parse query to merge and then `release`; none when there is none or it cannot be merged exactly |
| [`release`](#release) | what was read ahead for the file at `path` given back once its parse was merged, the tree, names and reports the merge copied going with the job's heap |

---

## Taken

```mach
pub rec Taken {
    tree:   *mach.front.ast.Ast;
    names:  *mach.base.intern.Interner;
    diags:  *mach.base.diagnostic.DiagnosticStore;
    status: mach.report.phase.PhaseStatus;
    fatal:  mach.front.parser.state.ParseFail;
    failed: bool;
}
```

a parse the walk took, which the parse query merges

## ReadAhead

```mach
pub rec ReadAhead {
    a:        *std.allocator.Allocator;
    spans:    *std.allocator.heap.Pool;
    sources:  mach.base.source.provider.Provider;
    crew:     mach.compile.workers.Crew;
    jobs:     Vector[*Job];
    by_path:  std.collections.map.MapBy[str, u32];
    slots:    Vector[u32];
    slot_job: *u32;
    width:    u32;
    pending:  Vector[u32];
}
```

a:       owns the jobs and their paths; spans: what each job's heap draws from
sources: the walk's sources, which locate on the owner and read on any thread
crew:    the threads; slots: free task indices, each in-flight job holding one (slot_job)
pending: proposed jobs not yet handed to a thread, newest last and handed first: the walk goes
         depth first, so it takes the first file a module names itself while the threads read
         on past it

### open

```mach
pub fun open(ra: *ReadAhead, s: *mach.session.Session, sources: mach.base.source.provider.Provider, threads: u32) bool
```

a read-ahead on `threads` threads for session `s` over the walk's `sources`, or false when it cannot be had (one thread, a
session with no span pool, storage, or a thread the host refused), holding nothing then. `ra` must
not move while it is open

### close

```mach
pub fun close(ra: *ReadAhead)
```

stop the threads, dropping what is still queued, and give back every job the walk left

### propose_uses

```mach
pub fun propose_uses(ra: *ReadAhead, path: str, tree: *mach.front.ast.Ast, text: str)
```

the files a parsed module's `use` and `fwd` paths may name proposed: each path and each of its
prefixes of two segments or more, as the load resolves a path to its longest prefix that is a
module. `sources` locates them, `text` is the module's source. proposing is best effort: a
path that cannot be spelled or located is left for the walk. a file read ahead proposed its uses
when it finished, so `path` names it to skip it

### text_for

```mach
pub fun text_for(ra: *ReadAhead, path: str) opt[mach.base.source.provider.Text]
```

the text the walk reads at `path`: what was read ahead, waiting for it if a thread is on it, or
none for the walk to read itself. the text stays the read-ahead's

### parse_for

```mach
pub fun parse_for(ra: *ReadAhead, path: str) opt[Taken]
```

the parse read ahead for the file at `path`, once the walk took its text, for the parse query to
merge and then `release`; none when there is none or it cannot be merged exactly

### release

```mach
pub fun release(ra: *ReadAhead, path: str)
```

what was read ahead for the file at `path` given back once its parse was merged, the tree, names
and reports the merge copied going with the job's heap

