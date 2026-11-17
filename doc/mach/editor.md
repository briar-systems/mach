[mach](README.md) / **editor**

# mach.editor

the analysis surface a language server builds on

an `EditorSession` serves the open buffers of a caller's session one request at a time: a
buffer is opened and updated, then analyzed to parse, resolve or sema, inside its project or
standalone. `project_init` analyzes a whole project with the open buffers read in place of
their files, and `parse_text` parses a text with no session at all. a result is an owned
envelope that outlives later edits, while the trees and tables it lends borrow the session.
nothing in the compiler reaches this module: mach-lsp does, and the `editor` artifact keeps
it built.

Source: [`src/editor.mach`](../../src/editor.mach)

Uses types from [`mach.base.diagnostic`](base/diagnostic.md), [`mach.base.fail`](base/fail.md), [`mach.base.intern`](base/intern.md), [`mach.base.source`](base/source.md), [`mach.compile`](compile.md), [`mach.compile.project`](compile/project.md), [`mach.compile.request`](compile/request.md), [`mach.front.ast`](front/ast.md), [`mach.front.resolve`](front/resolve.md), [`mach.front.sema.product`](front/sema/product.md), [`mach.report.phase`](report/phase.md), [`mach.session`](session.md), [`mach.session.query`](session/query.md), [`mach.target.resolved`](target/resolved.md).

| Type | | |
| :--- | :--- | :--- |
| [`EditorSession`](#editorsession) | `rec` | one serial facade over a borrowed session, with compact open-buffer slots |
| [`AnalysisRequest`](#analysisrequest) | `rec` | what one analyze call asks for |
| [`AnalysisResult`](#analysisresult) | `rec` | the owned envelope analyze returns: every diagnostic and source version it refers to, copied, so it outlives buffer edits, later analyses and the Session; the raw products (ast_of, resolve_of, sema_of) are not owned and borrow the serial view the result was taken in |
| [`ProjectSelection`](#projectselection) | `rec` | the build selection a project snapshot is analyzed under, over mach's own for the root; an empty field takes mach's choice for its axis, as the command line does when it names none reached only by mach-lsp |
| [`ProjectBuffer`](#projectbuffer) | `rec` | an open buffer's text, which a snapshot reads in place of the file at `path` reached only by mach-lsp |
| [`ProjectRequest`](#projectrequest) | `rec` | what one project_init call asks for; every string and the buffer array are borrowed only during the call reached only by mach-lsp |
| [`ProjectSnapshot`](#projectsnapshot) | `rec` | the whole project at a root analyzed for a tool, every artifact's closure with the open buffers' modules beside it |
| [`ImportRefusal`](#importrefusal) | `rec` | what the file of a module refused through an import shows: the path of its import that names a refused module, and the module-scope `$error` the refused imports from there lead to, which the build reports in its own module's file and not in this one reached only by mach-lsp |

| On `EditorSession` | |
| :--- | :--- |
| [`analyze`](#analyze) | run the frontend over an open buffer to the requested phase |
| [`init`](#init) | an editor facade over a caller-owned Session, borrowed for serial use; neither may move while the other holds its address, and there is no concurrent request support |
| [`open`](#open) | register a buffer's text under a path and return its FileId the text is loaded into the session SourceMap and set as the overlay for path, so a project load reads the buffer instead of the file on disk; marks the project dirty |
| [`update`](#update) | replace an open buffer's text and drop its cached analysis |
| [`dnit`](#dnit) | retire every editor-owned input together and free the editor's structural state |
| [`fail_dnit`](#fail_dnit) | release an operational failure returned by analyze or build |

| On `AnalysisRequest` | |
| :--- | :--- |
| [`analysis_request`](#analysis_request) | a request for one buffer at one phase on mach's own selection |

| On `AnalysisResult` | |
| :--- | :--- |
| [`analysis_dnit`](#analysis_dnit) | release an analysis result and everything it owns; a zero result is a no-op |
| [`ast_of`](#ast_of) | the AST of the analyzed buffer |
| [`resolve_of`](#resolve_of) | the resolve product, under ast_of's view rules; the phase must be at least compile.FRONTEND_RESOLVE |
| [`sema_of`](#sema_of) | the sema product, under ast_of's view rules; the phase must be driver.FRONTEND_SEMA reached only by mach-lsp |
| [`parse_text`](#parse_text) | parse a text on its own, with no buffer, project or session: the text is copied into the result as its source under FILE_NIL, and lexical and syntax errors become the result's diagnostics |
| [`same_shape`](#same_shape) | whether two results' trees have the same shape, as ast.same_shape compares them reached only by mach-lsp |

| On `ProjectRequest` | |
| :--- | :--- |
| [`project_request`](#project_request) | a request for the project at `root` to `phase`, on mach's own selection and with no buffers reached only by mach-lsp |
| [`project_init`](#project_init) | analyze the project at a root on the cell mach selects for it, over the union of its artifacts, with each open buffer's text read in place of its file |

| On `ProjectSnapshot` | |
| :--- | :--- |
| [`project_dnit`](#project_dnit) | release a snapshot: its project, the overlays it set on its Session and everything it owns |
| [`project_module_count`](#project_module_count) | how many modules the snapshot loaded; module indices run from 0 below it reached only by mach-lsp |
| [`project_module_by_path`](#project_module_by_path) | the module the snapshot loaded from the file at `path`, none when it loaded none reached only by mach-lsp |
| [`project_module_by_fqn`](#project_module_by_fqn) | the module the snapshot loaded under the fully qualified name `fqn`, none when it loaded none reached only by mach-lsp |
| [`project_module_fqn`](#project_module_fqn) | a module's fully qualified name, borrowed from the snapshot's Session reached only by mach-lsp |
| [`project_module_file`](#project_module_file) | the source file a module was read from, whose version the snapshot analyzed reached only by mach-lsp |
| [`project_ast_of`](#project_ast_of) | a module's AST, under ast_of's rules over the snapshot's Session; nil when the module has none reached only by mach-lsp |
| [`project_resolve_of`](#project_resolve_of) | a module's resolve product, under resolve_of's rules; nil when the module did not reach resolution reached only by mach-lsp |
| [`project_sema_of`](#project_sema_of) | a module's sema product, under sema_of's rules; nil when the module was not type checked reached only by mach-lsp |
| [`project_module_status`](#project_module_status) | a module's own phase standing, as the snapshot's analysis left it reached only by mach-lsp |
| [`project_import_refusal`](#project_import_refusal) | a module's import refusal, none when it is not refused through an import reached only by mach-lsp |
| [`project_module_is_root`](#project_module_is_root) | whether a module belongs to the project the snapshot was opened for, not to one of its dependencies reached only by mach-lsp |

| Functions | |
| :--- | :--- |
| [`is_identifier`](#is_identifier) | whether a text is exactly one identifier token, as the lexer reads it standalone reached only by mach-lsp |

---

## EditorSession

```mach
pub rec EditorSession {
    s:                 *mach.session.Session;
    alloc:             *std.allocator.Allocator;
    buffers:           *Buffer;
    buf_len:           u32;
    view_generation:   u64;
    active_request:    *AnalysisRequest;
    operation_failure: opt[mach.base.fail.Fail];

    proj_arena:      std.allocator.arena.Arena;
    proj_alloc:      std.allocator.Allocator;
    proj_loaded:     bool;
    proj:            mach.compile.project.Project;
    proj_root:       str;
    proj_dirty:      bool; # the closure itself is stale: a reload is due
    proj_edited:     std.collections.vector.Vector[mach.base.source.FileId]; # buffers whose text changed since the project loaded
    proj_loads:      u64; # closure loads performed, so a caller can see a reload happen
    proj_refreshes:  u64; # in-place refreshes performed, so a caller can see an edit stay cheap
    proj_phase:      mach.compile.FrontendPhase;
    proj_key:        ProjKey;
    proj_roots:      *mach.base.intern.StrId;
    proj_root_count: u32;
    proj_root_cap:   u32;

    root_retainers: Vector[RootRetainer];
}
```

one serial facade over a borrowed session, with compact open-buffer slots

### analyze

```mach
pub fun analyze(es: *EditorSession, req: *AnalysisRequest) res[AnalysisResult, mach.base.fail.Fail]
```

run the frontend over an open buffer to the requested phase. ok means the owned envelope
was produced; read result.status for the phase's standing and result.failure for an
operational project failure. the diagnostics own their source versions; the raw
products borrow this serial session view. request strings and vectors are borrowed only
during the call. an envelope that cannot be allocated is an operational failure owned
by the editor session, released with fail_dnit

| Parameter | |
| :--- | :--- |
| `es` | the editor session |
| `req` | the request; its file must be open and its phase at most compile.FRONTEND_SEMA |
| **returns** | the owned result, released with analysis_dnit, or the operational failure |

### init

```mach
pub fun init(s: *mach.session.Session) EditorSession
```

an editor facade over a caller-owned Session, borrowed for serial use; neither may move
while the other holds its address, and there is no concurrent request support. tear
down with dnit before the Session

| Parameter | |
| :--- | :--- |
| `s` | the Session and its allocator, both owned by the caller |

### open

```mach
pub fun open(es: *EditorSession, path: str, text: str) res[mach.base.source.FileId, mach.base.fail.Fail]
```

register a buffer's text under a path and return its FileId
the text is loaded into the session SourceMap and set as the overlay for path, so a
project load reads the buffer instead of the file on disk; marks the project dirty

| Parameter | |
| :--- | :--- |
| `es` | the editor session |
| `path` | the buffer's path; used to find the enclosing project and as the overlay key |
| `text` | the buffer's full text |
| **returns** | the FileId every other entry takes, or the SourceMap, slot, or overlay error |

### update

```mach
pub fun update(es: *EditorSession, fid: mach.base.source.FileId, text: str) res[bool, mach.base.fail.Fail]
```

replace an open buffer's text and drop its cached analysis

| Parameter | |
| :--- | :--- |
| `es` | the editor session |
| `fid` | the buffer's FileId from open |
| `text` | the new full text |
| **returns** | ok(true) when the text changed; ok(false) when the buffer is not open or the text is identical, in which case nothing is dropped; or the SourceMap or overlay error |

### dnit

```mach
pub fun dnit(es: *EditorSession) err[mach.base.fail.Fail]
```

retire every editor-owned input together and free the editor's structural state.
check the result: a failure leaves the editor and the Session intact for a retry, and
success leaves unrelated Session cache entries and stable source identities in place.
destroy the Session only after this succeeds; there is no separate close-all

| Parameter | |
| :--- | :--- |
| `es` | the editor session; nil is a no-op |

### fail_dnit

```mach
pub fun fail_dnit(es: *EditorSession, f: *mach.base.fail.Fail)
```

release an operational failure returned by analyze or build

| Parameter | |
| :--- | :--- |
| `es` | the editor session that returned the Fail |
| `f` | the Fail; nil is a no-op, and borrowed text (the out-of-memory message) is left alone |

## AnalysisRequest

```mach
pub rec AnalysisRequest {
    file:              mach.base.source.FileId;
    phase:             mach.compile.FrontendPhase;
    selection:         ProjectSelection;
    standalone_target: *mach.target.resolved.Target;
    standalone_build:  mach.compile.request.BuildRequest;
}
```

what one analyze call asks for

| Field | Type | |
| :--- | :--- | :--- |
| `file` | `mach.base.source.FileId` | an open buffer's FileId, the root of the analysis |
| `phase` | `mach.compile.FrontendPhase` | how far the frontend runs, compile.FRONTEND_PARSE, compile.FRONTEND_RESOLVE or compile.FRONTEND_SEMA |
| `selection` | `ProjectSelection` | the overrides of mach's selection for a file under a project's src, empty for none; its request is composed from the manifest as a project snapshot's is |
| `standalone_target` | `*mach.target.resolved.Target` | for a file under no manifest, a borrowed resolved target; nil selects the host target. a project's target selection is never replaced by it |
| `standalone_build` | `mach.compile.request.BuildRequest` | for a file under no manifest, the options it is analyzed with, since no profile composes them; a project's are never taken from it |

### analysis_request

```mach
pub fun analysis_request(file: mach.base.source.FileId, phase: mach.compile.FrontendPhase) AnalysisRequest
```

a request for one buffer at one phase on mach's own selection

## AnalysisResult

```mach
pub rec AnalysisResult {
    alloc:              *std.allocator.Allocator;
    editor:             *EditorSession;
    session:            *mach.session.Session;
    view_generation:    u64;
    session_generation: u64;
    query_operation:    u64;
    query_revision:     mach.session.query.Revision;
    file:               mach.base.source.FileId;
    source_revision:    u64;
    phase:              mach.compile.FrontendPhase;
    status:             mach.report.phase.PhaseStatus;
    failure:            opt[mach.base.fail.Fail];
    target:             AnalysisTarget;
    target_available:   bool;
    configuration:      mach.session.query.QueryOutput;
    project_root:       str;
    profile:            str;
    message:            str;
    diagnostics:        *mach.base.diagnostic.DiagnosticStore;
    sources:            Vector[AnalysisSource];
    tree:               *mach.front.ast.Ast;
    names:              *mach.base.intern.Interner;
}
```

the owned envelope analyze returns: every diagnostic and source version it refers to,
copied, so it outlives buffer edits, later analyses and the Session; the raw products
(ast_of, resolve_of, sema_of) are not owned and borrow the serial view the result was
taken in. release exactly once with analysis_dnit, never copy it as a second owner

| Field | Type | |
| :--- | :--- | :--- |
| `alloc` | `*std.allocator.Allocator` | owns every field below; must outlive the result |
| `status` | `mach.report.phase.PhaseStatus` | the phase's standing, accepted, rejected or internal; diagnostic counts never decide it |
| `failure` | `opt[mach.base.fail.Fail]` | an operational project failure with its fail.Fail category, absent when the frontend ran |
| `target` | `AnalysisTarget` | the target metadata the analysis ran under, read only when target_available |
| `target_available` | `bool` | false when the requested target context was not acquired; host metadata is never substituted |
| `diagnostics` | `*mach.base.diagnostic.DiagnosticStore` | the owned store, with related locations, fix edits and lost-diagnostic evidence |
| `sources` | `Vector[AnalysisSource]` | the owned source versions every diagnostic location refers to; analysis_source finds one by FileId |
| `tree` | `*mach.front.ast.Ast` | the owned tree of a parse_text result; nil for an analysis of a buffer |
| `names` | `*mach.base.intern.Interner` | the owned interner the names of a parse_text result's tree are ids in; nil for an analysis of a buffer, whose tree's names are its session's |

### analysis_dnit

```mach
pub fun analysis_dnit(result: *AnalysisResult)
```

release an analysis result and everything it owns; a zero result is a no-op. this
also ends the lifetime of every raw product borrowed from it, and a token stream from
tokenize must be freed before it because its bytes borrow the result's source

| Parameter | |
| :--- | :--- |
| `result` | the envelope; zeroed on return so a second call is a no-op |

### ast_of

```mach
pub fun ast_of(result: *AnalysisResult) res[*mach.front.ast.Ast, mach.base.fail.Fail]
```

the AST of the analyzed buffer. raw products borrow the current serial view: a later
open, update, close, dnit, analyze, build or query operation expires the view, after
which every checked accessor answers a Fail and a pointer already obtained must not be
dereferenced. a rejected parse can still expose a partial AST; a fatal acquisition
exposes nil. a parse_text result owns its tree, which stays valid until analysis_dnit
reached only by mach-lsp

| Parameter | |
| :--- | :--- |
| `result` | the envelope, whose EditorSession and Session must both still exist unless parse_text made it |
| **returns** | the borrowed AST, or the expiry failure |

### resolve_of

```mach
pub fun resolve_of(result: *AnalysisResult) res[*mach.front.resolve.ResolveResult, mach.base.fail.Fail]
```

the resolve product, under ast_of's view rules; the phase must be at least
compile.FRONTEND_RESOLVE. symbols may be SYMBOL_NIL or SYMBOL_REJECTED and must be checked before
indexing the symbol array
reached only by mach-lsp

| Parameter | |
| :--- | :--- |
| **returns** | the borrowed product, or the expiry or phase failure |

### sema_of

```mach
pub fun sema_of(result: *AnalysisResult) res[*mach.front.sema.product.SemaResult, mach.base.fail.Fail]
```

the sema product, under ast_of's view rules; the phase must be driver.FRONTEND_SEMA
reached only by mach-lsp

| Parameter | |
| :--- | :--- |
| **returns** | the borrowed product, or the expiry or phase failure |

### parse_text

```mach
pub fun parse_text(alloc: *std.allocator.Allocator, text: str) res[AnalysisResult, mach.base.fail.Fail]
```

parse a text on its own, with no buffer, project or session: the text is copied into the
result as its source under FILE_NIL, and lexical and syntax errors become the result's
diagnostics. ok means the owned envelope was produced; read result.status for the parse's
standing and result.failure for a fatal parse, whose tree is left empty. the tree is
read with ast_of and lives until analysis_dnit, which releases the result exactly once.
resolve_of and sema_of answer a phase failure
reached only by mach-lsp

| Parameter | |
| :--- | :--- |
| `alloc` | owns the result; must outlive it |
| `text` | the source, borrowed only during the call |
| **returns** | the owned result, or the allocation failure |

### same_shape

```mach
pub fun same_shape(x: *AnalysisResult, y: *AnalysisResult) res[bool, mach.base.fail.Fail]
```

whether two results' trees have the same shape, as ast.same_shape compares them
reached only by mach-lsp

| Parameter | |
| :--- | :--- |
| **returns** | whether the shapes match, or either tree's expiry failure |

## ProjectSelection

```mach
pub rec ProjectSelection {
    artifact: str;
    target:   str;
    profile:  str;
}
```

the build selection a project snapshot is analyzed under, over mach's own for the root; an
empty field takes mach's choice for its axis, as the command line does when it names none
reached only by mach-lsp

| Field | Type | |
| :--- | :--- | :--- |
| `artifact` | `str` | the declared artifact name, or "" |
| `target` | `str` | the declared target name, or "" |
| `profile` | `str` | the declared profile name, or "" |

## ProjectBuffer

```mach
pub rec ProjectBuffer {
    path: str;
    text: str;
}
```

an open buffer's text, which a snapshot reads in place of the file at `path`
reached only by mach-lsp

## ProjectRequest

```mach
pub rec ProjectRequest {
    root:         str;
    phase:        mach.compile.FrontendPhase;
    selection:    ProjectSelection;
    buffers:      *ProjectBuffer;
    buffer_count: u32;
}
```

what one project_init call asks for; every string and the buffer array are borrowed only
during the call
reached only by mach-lsp

| Field | Type | |
| :--- | :--- | :--- |
| `root` | `str` | the project's directory, the one holding its mach.toml |
| `phase` | `mach.compile.FrontendPhase` | how far the frontend runs, compile.FRONTEND_PARSE, compile.FRONTEND_RESOLVE or compile.FRONTEND_SEMA |
| `selection` | `ProjectSelection` | the overrides of mach's selection, empty for none |
| `buffers` | `*ProjectBuffer` | the open buffers, nil with buffer_count 0 |
| `buffer_count` | `u32` | how many buffers |

### project_request

```mach
pub fun project_request(root: str, phase: mach.compile.FrontendPhase) ProjectRequest
```

a request for the project at `root` to `phase`, on mach's own selection and with no buffers
reached only by mach-lsp

### project_init

```mach
pub fun project_init(s: *mach.session.Session, req: *ProjectRequest) res[ProjectSnapshot, mach.base.fail.Fail]
```

analyze the project at a root on the cell mach selects for it, over the union of its artifacts,
with each open buffer's text read in place of its file. the request is composed from the
manifest and the selection as `mach check` composes it, so a tool never restates a build. the
Session is any the caller owns, not one an EditorSession serves, so a snapshot can be opened
off the thread that serves open buffers; it holds one snapshot at a time. ok means the owned
envelope was produced: read status for the standing and failure for a project that could not
load. the buffers' overlays stay set on the Session until project_dnit
reached only by mach-lsp

| Parameter | |
| :--- | :--- |
| `s` | the Session; it must outlive the snapshot |
| `req` | the request |
| **returns** | the snapshot, released with project_dnit, or the operational failure, owned through the Session's allocator and released with fail.dnit |

## ProjectSnapshot

```mach
pub rec ProjectSnapshot {
    alloc:           *std.allocator.Allocator;
    session:         *mach.session.Session;
    phase:           mach.compile.FrontendPhase;
    loaded:          bool;
    project:         mach.compile.project.Project;
    overlays:        Vector[str];
    query_operation: u64;
    query_revision:  mach.session.query.Revision;
    status:          mach.report.phase.PhaseStatus;
    failure:         opt[mach.base.fail.Fail];
    message:         str;
    diagnostics:     *mach.base.diagnostic.DiagnosticStore;
    sources:         Vector[AnalysisSource];
    root:            str;
    key:             ProjKey;
}
```

the whole project at a root analyzed for a tool, every artifact's closure with the open
buffers' modules beside it. the envelope fields are owned as AnalysisResult's are; the module
products borrow the Session the snapshot was opened in and expire with its next query
operation. release exactly once with project_dnit, never copy it as a second owner
reached only by mach-lsp

| Field | Type | |
| :--- | :--- | :--- |
| `alloc` | `*std.allocator.Allocator` | the Session's allocator, which owns every field below |
| `session` | `*mach.session.Session` | the Session the snapshot was opened in and whose overlays it holds |
| `phase` | `mach.compile.FrontendPhase` | the phase it was opened to |
| `loaded` | `bool` | whether `project` holds a project; false when the project could not be loaded |
| `status` | `mach.report.phase.PhaseStatus` | the frontend's standing over every module, accepted, rejected or internal |
| `failure` | `opt[mach.base.fail.Fail]` | the operational failure that kept the project from loading, absent when it loaded |
| `message` | `str` | the failure's text, or the internal standing's, "" for neither |
| `diagnostics` | `*mach.base.diagnostic.DiagnosticStore` | the owned store, with related locations, fix edits and lost-diagnostic evidence |
| `sources` | `Vector[AnalysisSource]` | the owned source versions every diagnostic location refers to |
| `root` | `str` | the owned copy of the request's root, which the loaded project's request borrows |
| `key` | `ProjKey` | the owned copies of the request's selection strings, which the loaded project borrows |

### project_dnit

```mach
pub fun project_dnit(snap: *ProjectSnapshot) err[mach.base.fail.Fail]
```

release a snapshot: its project, the overlays it set on its Session and everything it owns. a
zero snapshot is a no-op. check the result: a failure leaves the snapshot intact for a retry
reached only by mach-lsp

| Parameter | |
| :--- | :--- |
| `snap` | the snapshot; zeroed on success so a second call is a no-op |

### project_module_count

```mach
pub fun project_module_count(snap: *ProjectSnapshot) u32
```

how many modules the snapshot loaded; module indices run from 0 below it
reached only by mach-lsp

### project_module_by_path

```mach
pub fun project_module_by_path(snap: *ProjectSnapshot, path: str) opt[u32]
```

the module the snapshot loaded from the file at `path`, none when it loaded none
reached only by mach-lsp

### project_module_by_fqn

```mach
pub fun project_module_by_fqn(snap: *ProjectSnapshot, fqn: str) opt[u32]
```

the module the snapshot loaded under the fully qualified name `fqn`, none when it loaded none
reached only by mach-lsp

### project_module_fqn

```mach
pub fun project_module_fqn(snap: *ProjectSnapshot, index: u32) res[str, mach.base.fail.Fail]
```

a module's fully qualified name, borrowed from the snapshot's Session
reached only by mach-lsp

### project_module_file

```mach
pub fun project_module_file(snap: *ProjectSnapshot, index: u32) res[mach.base.source.FileId, mach.base.fail.Fail]
```

the source file a module was read from, whose version the snapshot analyzed
reached only by mach-lsp

### project_ast_of

```mach
pub fun project_ast_of(snap: *ProjectSnapshot, index: u32) res[*mach.front.ast.Ast, mach.base.fail.Fail]
```

a module's AST, under ast_of's rules over the snapshot's Session; nil when the module has none
reached only by mach-lsp

### project_resolve_of

```mach
pub fun project_resolve_of(snap: *ProjectSnapshot, index: u32) res[*mach.front.resolve.ResolveResult, mach.base.fail.Fail]
```

a module's resolve product, under resolve_of's rules; nil when the module did not reach resolution
reached only by mach-lsp

### project_sema_of

```mach
pub fun project_sema_of(snap: *ProjectSnapshot, index: u32) res[*mach.front.sema.product.SemaResult, mach.base.fail.Fail]
```

a module's sema product, under sema_of's rules; nil when the module was not type checked
reached only by mach-lsp

### project_module_status

```mach
pub fun project_module_status(snap: *ProjectSnapshot, index: u32) res[mach.report.phase.PhaseStatus, mach.base.fail.Fail]
```

a module's own phase standing, as the snapshot's analysis left it
reached only by mach-lsp

### project_import_refusal

```mach
pub fun project_import_refusal(snap: *ProjectSnapshot, index: u32) res[opt[ImportRefusal], mach.base.fail.Fail]
```

a module's import refusal, none when it is not refused through an import
reached only by mach-lsp

### project_module_is_root

```mach
pub fun project_module_is_root(snap: *ProjectSnapshot, index: u32) res[bool, mach.base.fail.Fail]
```

whether a module belongs to the project the snapshot was opened for, not to one of its dependencies
reached only by mach-lsp

## ImportRefusal

```mach
pub rec ImportRefusal {
    import: mach.base.source.Span;
    cause:  *mach.base.diagnostic.Diagnostic;
}
```

what the file of a module refused through an import shows: the path of its import that names
a refused module, and the module-scope `$error` the refused imports from there lead to, which
the build reports in its own module's file and not in this one
reached only by mach-lsp

| Field | Type | |
| :--- | :--- | :--- |
| `import` | `mach.base.source.Span` | the import's path, in the module's file |
| `cause` | `*mach.base.diagnostic.Diagnostic` | the `$error`, borrowed from the snapshot's diagnostics |

## Functions

### is_identifier

```mach
pub fun is_identifier(alloc: *std.allocator.Allocator, text: str) res[bool, mach.base.fail.Fail]
```

whether a text is exactly one identifier token, as the lexer reads it standalone
reached only by mach-lsp

| Parameter | |
| :--- | :--- |
| `alloc` | scratch for the lex, freed before return |
| `text` | the candidate, borrowed only during the call |
| **returns** | whether it is one identifier token, or the failure that stopped the lex |

