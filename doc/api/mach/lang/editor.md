# mach.lang.editor

## rec Buffer

```mach
pub rec Buffer;
```

one reusable slot for a currently open file

## rec EditorSession

```mach
pub rec EditorSession;
```

one serial facade over a borrowed session, with compact open-buffer slots

## def AnalysisPhase

```mach
pub def AnalysisPhase: driver.FrontendPhase
```

## val PHASE_PARSE

```mach
pub val PHASE_PARSE:   AnalysisPhase = driver.FRONTEND_PARSE
```

## val PHASE_RESOLVE

```mach
pub val PHASE_RESOLVE: AnalysisPhase = driver.FRONTEND_RESOLVE
```

## val PHASE_SEMA

```mach
pub val PHASE_SEMA:    AnalysisPhase = driver.FRONTEND_SEMA
```

## rec AnalysisRequest

```mach
pub rec AnalysisRequest;
```

what one analyze call asks for

file: an open buffer's FileId, the root of the analysis
phase: how far the frontend runs, PHASE_PARSE, PHASE_RESOLVE or PHASE_SEMA
build: the selection and effective options a file under a project's src is analyzed
                   with; the same boundary the driver uses, its profile options already composed
standalone_target: for a file under no manifest, a borrowed resolved target whose
                   registry-owned definitions outlive the Session; nil selects the registered host target.
                   a project's target selection is never replaced by it

## rec AnalysisTarget

```mach
pub rec AnalysisTarget;
```

## rec AnalysisSource

```mach
pub rec AnalysisSource;
```

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
failure: an operational project failure with its outcome.Fail category, absent when the
                  frontend ran
target: the target metadata the analysis ran under, read only when target_available
target_available: false when the requested target context was not acquired; host
                  metadata is never substituted
diagnostics: the owned store, with related locations, fix edits and lost-diagnostic evidence
sources: the owned source versions every diagnostic location refers to; analysis_source
                  finds one by FileId

## fun analysis_request

```mach
pub fun analysis_request(file: source.FileId, phase: AnalysisPhase) AnalysisRequest;
```

a request for one buffer at one phase with the default build selection

## fun analysis_dnit

```mach
pub fun analysis_dnit(result: *AnalysisResult);
```

release an analysis result and everything it owns; a zero result is a no-op. this
also ends the lifetime of every raw product borrowed from it, and a token stream from
tokenize must be freed before it because its bytes borrow the result's source

result: the envelope; zeroed on return so a second call is a no-op

## fun analysis_source

```mach
pub fun analysis_source(result: *AnalysisResult, file: source.FileId) opt[*source.SourceFile];
```

the owned source version a diagnostic location refers to; use source.position on it
rather than the Session's current text, which may have moved on

result: the envelope
file: the FileId from a diagnostic location
ret: the owned SourceFile, valid until analysis_dnit; none when the result carries no
        version of that file

## fun ast_of

```mach
pub fun ast_of(result: *AnalysisResult) res[*ast.Ast, fail.Fail];
```

the AST of the analyzed buffer. raw products borrow the current serial view: a later
open, update, close, dnit, analyze, build or query operation expires the view, after
which every checked accessor answers a Fail and a pointer already obtained must not be
dereferenced. a rejected parse can still expose a partial AST; a fatal acquisition
exposes nil

result: the envelope, whose EditorSession and Session must both still exist
ret: the borrowed AST, or the expiry failure

## fun resolve_of

```mach
pub fun resolve_of(result: *AnalysisResult) res[*resolve.ResolveResult, fail.Fail];
```

the resolve product, under ast_of's view rules; the phase must be at least
PHASE_RESOLVE. symbols may be SYMBOL_NIL or SYMBOL_REJECTED and must be checked before
indexing the symbol array

ret: the borrowed product, or the expiry or phase failure

## fun sema_of

```mach
pub fun sema_of(result: *AnalysisResult) res[*context.SemaResult, fail.Fail];
```

the sema product, under ast_of's view rules; the phase must be PHASE_SEMA

ret: the borrowed product, or the expiry or phase failure

## fun analyze

```mach
pub fun analyze(es: *EditorSession, req: *AnalysisRequest) res[AnalysisResult, outcome.Fail];
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

## fun expr_type_of

```mach
pub fun expr_type_of(result: *AnalysisResult, eid: id.ExprId) res[type.TypeId, fail.Fail];
```

the type sema assigned to an expression node; the same view and phase checks as
sema_of, and the node id is meaningful only with the AST of this result

ret: the TypeId (TYPE_NIL for an untyped node), or the expiry or phase failure

## fun decl_type_of

```mach
pub fun decl_type_of(result: *AnalysisResult, did: id.DeclId) res[type.TypeId, fail.Fail];
```

the type sema assigned to a declaration node, under expr_type_of's rules

## fun resolved_type_of

```mach
pub fun resolved_type_of(result: *AnalysisResult, tid: id.TypeId) res[type.TypeId, fail.Fail];
```

the resolved type behind a type node, under expr_type_of's rules

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
pub fun open(es: *EditorSession, path: str, text: str) res[source.FileId, fail.Fail];
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
pub fun update(es: *EditorSession, fid: source.FileId, text: str) res[bool, fail.Fail];
```

replace an open buffer's text and drop its cached analysis

es: the editor session
fid: the buffer's FileId from open
text: the new full text
ret: ok(true) when the text changed; ok(false) when the buffer is not open or the text
      is identical, in which case nothing is dropped; or the SourceMap or overlay error

## fun close

```mach
pub fun close(es: *EditorSession, fid: source.FileId) res[bool, fail.Fail];
```

close an open buffer: its overlay, cached dependent products and source payload go;
file identity and path metadata survive, and reopening the same canonical path returns
the same FileId with a fresh revision. expires every raw analysis view. every fallible
allocation is prepared before anything is published, so a failure leaves the open
buffers and current views intact and may be retried

es: the editor session
fid: the buffer's FileId
ret: ok(true) when a buffer was closed, ok(false) when none was open under fid

## fun dnit

```mach
pub fun dnit(es: *EditorSession) err[fail.Fail];
```

retire every editor-owned input together and free the editor's structural state.
check the result: a failure leaves the editor and the Session intact for a retry, and
success leaves unrelated Session cache entries and stable source identities in place.
destroy the Session only after this succeeds; there is no separate close-all

es: the editor session; nil is a no-op

## fun tokenize

```mach
pub fun tokenize(result: *AnalysisResult) res[lexer.TokenStream, fail.Fail];
```

lex the result's owned source version without touching the Session. the caller owns
the token storage and frees it with lexer.dnit(stream, result.alloc); its bytes borrow
the result, so free the stream before analysis_dnit

result: the envelope
ret: the token stream, or a failure when the result carries no owned source version

## fun build

```mach
pub fun build(es: *EditorSession, req: *request.BuildRequest) res[outcome.BuildOutcome, outcome.Fail];
```

run a full build of a project through the warm engine, reading open buffers through the
session's overlays. the manifest is loaded fresh on every call

es: the editor session
req: the build request; project_root must name the project directory
ret: the build outcome, or a Fail whose message is a copy owned by the editor session;
     release it with fail_dnit

## fun fail_dnit

```mach
pub fun fail_dnit(es: *EditorSession, f: *outcome.Fail);
```

release an operational failure returned by analyze or build

es: the editor session that returned the Fail
f: the Fail; nil, a reported Fail, an empty message, and the out-of-memory message are no-ops

