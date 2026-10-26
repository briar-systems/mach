# mach.lang.editor

## rec EditorSession

```mach
pub rec EditorSession;
```

one serial facade over a borrowed session, with compact open-buffer slots

## def AnalysisPhase

```mach
pub def AnalysisPhase: driver.FrontendPhase
```

reached only by mach-lsp

## val PHASE_PARSE

```mach
pub val PHASE_PARSE: AnalysisPhase = driver.FRONTEND_PARSE
```

## val PHASE_RESOLVE

```mach
pub val PHASE_RESOLVE: AnalysisPhase = driver.FRONTEND_RESOLVE
```

reached only by mach-lsp

## val PHASE_SEMA

```mach
pub val PHASE_SEMA: AnalysisPhase = driver.FRONTEND_SEMA
```

reached only by mach-lsp

## rec AnalysisRequest

```mach
pub rec AnalysisRequest;
```

what one analyze call asks for

file: an open buffer's FileId, the root of the analysis
phase: how far the frontend runs, PHASE_PARSE, PHASE_RESOLVE or PHASE_SEMA
selection: the overrides of mach's selection for a file under a project's src, empty for
                   none; its request is composed from the manifest as a project snapshot's is
standalone_target: for a file under no manifest, a borrowed resolved target whose
                   registry-owned definitions outlive the Session; nil selects the registered host target.
                   a project's target selection is never replaced by it
standalone_build: for a file under no manifest, the options it is analyzed with, since no profile
                   composes them; a project's are never taken from it

## rec AnalysisResult

```mach
pub rec AnalysisResult;
```

the owned envelope analyze returns: every diagnostic and source version it refers to,
copied, so it outlives buffer edits, later analyses and the Session; the raw products
(ast_of, resolve_of, sema_of) are not owned and borrow the serial view the result was
taken in. release exactly once with analysis_dnit, never copy it as a second owner

alloc: owns every field below; must outlive the result
status: the phase's standing, accepted, rejected or internal; diagnostic counts never decide it
failure: an operational project failure with its fail.Fail category, absent when the
                  frontend ran
target: the target metadata the analysis ran under, read only when target_available
target_available: false when the requested target context was not acquired; host
                  metadata is never substituted
diagnostics: the owned store, with related locations, fix edits and lost-diagnostic evidence
sources: the owned source versions every diagnostic location refers to; analysis_source
                  finds one by FileId
tree: the owned tree of a parse_text result; nil for an analysis of a buffer

## fun analysis_request

```mach
pub fun analysis_request(file: lang_source.FileId, phase: AnalysisPhase) AnalysisRequest;
```

a request for one buffer at one phase on mach's own selection

## fun analysis_dnit

```mach
pub fun analysis_dnit(result: *AnalysisResult);
```

release an analysis result and everything it owns; a zero result is a no-op. this
also ends the lifetime of every raw product borrowed from it, and a token stream from
tokenize must be freed before it because its bytes borrow the result's source

result: the envelope; zeroed on return so a second call is a no-op

## fun ast_of

```mach
pub fun ast_of(result: *AnalysisResult) res[*ast.Ast, fail.Fail];
```

the AST of the analyzed buffer. raw products borrow the current serial view: a later
open, update, close, dnit, analyze, build or query operation expires the view, after
which every checked accessor answers a Fail and a pointer already obtained must not be
dereferenced. a rejected parse can still expose a partial AST; a fatal acquisition
exposes nil. a parse_text result owns its tree, which stays valid until analysis_dnit
reached only by mach-lsp

result: the envelope, whose EditorSession and Session must both still exist unless
        parse_text made it
ret: the borrowed AST, or the expiry failure

## fun resolve_of

```mach
pub fun resolve_of(result: *AnalysisResult) res[*resolve.ResolveResult, fail.Fail];
```

the resolve product, under ast_of's view rules; the phase must be at least
PHASE_RESOLVE. symbols may be SYMBOL_NIL or SYMBOL_REJECTED and must be checked before
indexing the symbol array
reached only by mach-lsp

ret: the borrowed product, or the expiry or phase failure

## fun sema_of

```mach
pub fun sema_of(result: *AnalysisResult) res[*fe_sema.SemaResult, fail.Fail];
```

the sema product, under ast_of's view rules; the phase must be PHASE_SEMA
reached only by mach-lsp

ret: the borrowed product, or the expiry or phase failure

## fun analyze

```mach
pub fun analyze(es: *EditorSession, req: *AnalysisRequest) res[AnalysisResult, fail.Fail];
```

run the frontend over an open buffer to the requested phase. ok means the owned envelope
was produced; read result.status for the phase's standing and result.failure for an
operational project failure. the diagnostics own their source versions; the raw
products borrow this serial session view. request strings and vectors are borrowed only
during the call. an envelope that cannot be allocated is an operational failure owned
by the editor session, released with fail_dnit

es: the editor session
req: the request; its file must be open and its phase at most PHASE_SEMA
ret: the owned result, released with analysis_dnit, or the operational failure

## rec ProjectSelection

```mach
pub rec ProjectSelection;
```

the build selection a project snapshot is analyzed under, over mach's own for the root; an
empty field takes mach's choice for its axis, as the command line does when it names none
reached only by mach-lsp

artifact: the declared artifact name, or ""
target: the declared target name, or ""
profile: the declared profile name, or ""

## rec ProjectBuffer

```mach
pub rec ProjectBuffer;
```

an open buffer's text, which a snapshot reads in place of the file at `path`
reached only by mach-lsp

## rec ProjectRequest

```mach
pub rec ProjectRequest;
```

what one project_open call asks for; every string and the buffer array are borrowed only
during the call
reached only by mach-lsp

root: the project's directory, the one holding its mach.toml
phase: how far the frontend runs, PHASE_PARSE, PHASE_RESOLVE or PHASE_SEMA
selection: the overrides of mach's selection, empty for none
buffers: the open buffers, nil with buffer_count 0
buffer_count: how many buffers

## rec ProjectSnapshot

```mach
pub rec ProjectSnapshot;
```

the whole project at a root analyzed for a tool, every artifact's closure with the open
buffers' modules beside it. the envelope fields are owned as AnalysisResult's are; the module
products borrow the Session the snapshot was opened in and expire with its next query
operation. release exactly once with project_dnit, never copy it as a second owner
reached only by mach-lsp

alloc: the Session's allocator, which owns every field below
session: the Session the snapshot was opened in and whose overlays it holds
phase: the phase it was opened to
loaded: whether `project` holds a project; false when the project could not be loaded
status: the frontend's standing over every module, accepted, rejected or internal
failure: the operational failure that kept the project from loading, absent when it loaded
message: the failure's text, or the internal standing's, "" for neither
diagnostics: the owned store, with related locations, fix edits and lost-diagnostic evidence
sources: the owned source versions every diagnostic location refers to
root: the owned copy of the request's root, which the loaded project's request borrows
key: the owned copies of the request's selection strings, which the loaded project borrows

## fun project_request

```mach
pub fun project_request(root: str, phase: AnalysisPhase) ProjectRequest;
```

a request for the project at `root` to `phase`, on mach's own selection and with no buffers
reached only by mach-lsp

## fun project_open

```mach
pub fun project_open(s: *session.Session, req: *ProjectRequest) res[ProjectSnapshot, fail.Fail];
```

analyze the project at a root on the cell mach selects for it, over the union of its artifacts,
with each open buffer's text read in place of its file. the request is composed from the
manifest and the selection as `mach check` composes it, so a tool never restates a build. the
Session is any the caller owns, not one an EditorSession serves, so a snapshot can be opened
off the thread that serves open buffers; it holds one snapshot at a time. ok means the owned
envelope was produced: read status for the standing and failure for a project that could not
load. the buffers' overlays stay set on the Session until project_dnit
reached only by mach-lsp

s: the Session; it must outlive the snapshot
req: the request
ret: the snapshot, released with project_dnit, or the operational failure, owned through the
     Session's allocator and released with fail.dnit

## fun project_dnit

```mach
pub fun project_dnit(snap: *ProjectSnapshot) err[fail.Fail];
```

release a snapshot: its project, the overlays it set on its Session and everything it owns. a
zero snapshot is a no-op. check the result: a failure leaves the snapshot intact for a retry
reached only by mach-lsp

snap: the snapshot; zeroed on success so a second call is a no-op

## fun project_module_count

```mach
pub fun project_module_count(snap: *ProjectSnapshot) u32;
```

how many modules the snapshot loaded; module indices run from 0 below it
reached only by mach-lsp

## fun project_module_by_path

```mach
pub fun project_module_by_path(snap: *ProjectSnapshot, path: str) opt[u32];
```

the module the snapshot loaded from the file at `path`, none when it loaded none
reached only by mach-lsp

## fun project_module_by_fqn

```mach
pub fun project_module_by_fqn(snap: *ProjectSnapshot, fqn: str) opt[u32];
```

the module the snapshot loaded under the fully qualified name `fqn`, none when it loaded none
reached only by mach-lsp

## fun project_module_fqn

```mach
pub fun project_module_fqn(snap: *ProjectSnapshot, index: u32) res[str, fail.Fail];
```

a module's fully qualified name, borrowed from the snapshot's Session
reached only by mach-lsp

## fun project_module_file

```mach
pub fun project_module_file(snap: *ProjectSnapshot, index: u32) res[lang_source.FileId, fail.Fail];
```

the source file a module was read from, whose version the snapshot analyzed
reached only by mach-lsp

## fun project_ast_of

```mach
pub fun project_ast_of(snap: *ProjectSnapshot, index: u32) res[*ast.Ast, fail.Fail];
```

a module's AST, under ast_of's rules over the snapshot's Session; nil when the module has none
reached only by mach-lsp

## fun project_resolve_of

```mach
pub fun project_resolve_of(snap: *ProjectSnapshot, index: u32) res[*resolve.ResolveResult, fail.Fail];
```

a module's resolve product, under resolve_of's rules; nil when the module did not reach resolution
reached only by mach-lsp

## fun project_sema_of

```mach
pub fun project_sema_of(snap: *ProjectSnapshot, index: u32) res[*fe_sema.SemaResult, fail.Fail];
```

a module's sema product, under sema_of's rules; nil when the module was not type checked
reached only by mach-lsp

## fun project_module_status

```mach
pub fun project_module_status(snap: *ProjectSnapshot, index: u32) res[fail.PhaseStatus, fail.Fail];
```

a module's own phase standing, as the snapshot's analysis left it
reached only by mach-lsp

## fun project_module_is_root

```mach
pub fun project_module_is_root(snap: *ProjectSnapshot, index: u32) res[bool, fail.Fail];
```

whether a module belongs to the project the snapshot was opened for, not to one of its dependencies
reached only by mach-lsp

## fun init

```mach
pub fun init(s: *session.Session) EditorSession;
```

an editor facade over a caller-owned Session, borrowed for serial use; neither may move
while the other holds its address, and there is no concurrent request support. tear
down with dnit before the Session

s: the Session and its allocator, both owned by the caller

## fun open

```mach
pub fun open(es: *EditorSession, path: str, text: str) res[lang_source.FileId, fail.Fail];
```

register a buffer's text under a path and return its FileId
the text is loaded into the session SourceMap and set as the overlay for path, so a
project load reads the buffer instead of the file on disk; marks the project dirty

es: the editor session
path: the buffer's path; used to find the enclosing project and as the overlay key
text: the buffer's full text
ret: the FileId every other entry takes, or the SourceMap, slot, or overlay error

## fun update

```mach
pub fun update(es: *EditorSession, fid: lang_source.FileId, text: str) res[bool, fail.Fail];
```

replace an open buffer's text and drop its cached analysis

es: the editor session
fid: the buffer's FileId from open
text: the new full text
ret: ok(true) when the text changed; ok(false) when the buffer is not open or the text
      is identical, in which case nothing is dropped; or the SourceMap or overlay error

## fun dnit

```mach
pub fun dnit(es: *EditorSession) err[fail.Fail];
```

retire every editor-owned input together and free the editor's structural state.
check the result: a failure leaves the editor and the Session intact for a retry, and
success leaves unrelated Session cache entries and stable source identities in place.
destroy the Session only after this succeeds; there is no separate close-all

es: the editor session; nil is a no-op

## fun parse_text

```mach
pub fun parse_text(alloc: *A.Allocator, text: str) res[AnalysisResult, fail.Fail];
```

parse a text on its own, with no buffer, project or session: the text is copied into the
result as its source under FILE_NIL, and lexical and syntax errors become the result's
diagnostics. ok means the owned envelope was produced; read result.status for the parse's
standing and result.failure for a fatal parse, whose tree is left empty. the tree is
read with ast_of and lives until analysis_dnit, which releases the result exactly once.
resolve_of and sema_of answer a phase failure
reached only by mach-lsp

alloc: owns the result; must outlive it
text: the source, borrowed only during the call
ret: the owned result, or the allocation failure

## fun same_shape

```mach
pub fun same_shape(x: *AnalysisResult, y: *AnalysisResult) res[bool, fail.Fail];
```

whether two results' trees have the same shape, as ast.same_shape compares them
reached only by mach-lsp

ret: whether the shapes match, or either tree's expiry failure

## fun is_identifier

```mach
pub fun is_identifier(alloc: *A.Allocator, text: str) res[bool, fail.Fail];
```

whether a text is exactly one identifier token, as the lexer reads it standalone
reached only by mach-lsp

alloc: scratch for the lex, freed before return
text: the candidate, borrowed only during the call
ret: whether it is one identifier token, or the failure that stopped the lex

## fun fail_dnit

```mach
pub fun fail_dnit(es: *EditorSession, f: *fail.Fail);
```

release an operational failure returned by analyze or build

es: the editor session that returned the Fail
f: the Fail; nil is a no-op, and borrowed text (the out-of-memory message) is left alone

