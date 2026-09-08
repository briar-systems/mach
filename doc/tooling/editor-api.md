# Editor API (`mach.lang.editor`)

The editor facade analyzes unsaved buffers through the compiler's frontend and
query cache. The caller owns a `session.Session` and its allocator. One
`EditorSession` borrows that Session for serial use. Neither object may move
while its address is borrowed. There is no concurrent request support.

## Buffer ownership

`open(es, path, text)` copies an overlay and source payload and returns a stable
`FileId`. `update(es, file, text)` replaces an open buffer. Identical text is a
no-op. `close(es, file)` removes its overlay, cached dependent products and
source payload. Closing an unopened file returns `ok(false)`.

File identity and path metadata survive close. Reopening the same canonical
path returns the same `FileId` with a fresh source revision. Buffer slots are
reused independently of FileIds and grow to peak simultaneous opens. A closed
buffer stops contributing an extra project root. If another module imports
that file, a later project analysis reads its current disk contents.

Open, update, close and teardown prepare every fallible allocation before
publishing their changes. An error preserves the open buffers and current
views, including their overlays and project state. Internal table capacity can
remain reserved after a refusal. Retry is permitted.

`dnit(es) -> Result[Void, str]` retires all editor-owned inputs together, then
frees the editor's structural state. Check this result. Failure leaves the
editor and Session intact. Success leaves unrelated Session cache entries and
stable source identities intact. Destroy the Session only after successful
editor teardown. There is no separate public close-all protocol.

## Analysis request and owned result

Create an `AnalysisRequest` with `analysis_request(file, phase)` and call
`analyze(es, request) -> Result[AnalysisResult, outcome.Fail]`.

| Phase | Work |
|---|---|
| `PHASE_PARSE` | Lex and parse the selected source roots. No import traversal, gate evaluation, name resolution or type checking. |
| `PHASE_RESOLVE` | Discover active dependencies and resolve names and gates. No ordinary sema pass. |
| `PHASE_SEMA` | Resolve and type-check. |

For a file under its nearest manifest's `src` directory, `request.build` uses
the existing `BuildRequest` selection and effective options. This is the same
project/configuration boundary as the driver. Opening that project still
validates its configuration and dependency manifests. Parse-only analysis does
not read dependency source files. Currently open files belonging to that same
nearest project form the extra analysis roots. An open nested project's file
does not become an extra root of its parent project.

For a standalone file, `standalone_target` optionally borrows a resolved
`target.Target`. Its registry-owned definitions must remain live with the
Session. Omitting it selects the registered host target. A project target name
cannot select a standalone target. Conversely, a standalone target cannot
replace a project's target selection. Effective build mode and PIE come from
`request.build`, whose profile options must already be composed. Request
strings and vectors are borrowed only during the call.

A successful `Result` means the owned analysis envelope was produced. Inspect
`AnalysisResult.status` for accepted, rejected or internal phase status.
Operational project failures retain their `outcome.Fail` category in
`result.failure`. Diagnostic counts do not determine phase status. A rejected
standalone parse can still expose a partial AST. A fatal project acquisition
can leave no raw product, while retaining the owned failure diagnostics.

The result owns:

- Its diagnostic store, including related locations, fix edits and lost-diagnostic evidence.
- The requested source version and every other source version referenced by those locations.
- A copied failure message, source revision, requested phase and target metadata.
- For an opened project, its root, resolved profile name and the existing canonical build-configuration bytes.

`target_available` is false when the requested target context was not acquired.
The API does not substitute host metadata for a failed project target.

Release each result exactly once with `analysis_dnit`. Do not copy it as an
independent owner. Its allocator must outlive it. Owned diagnostics, source
versions and metadata remain usable after buffer update/close, subsequent
analysis and Session destruction. `analysis_source(result, file)` finds the
owned `SourceFile` for a diagnostic location. Use `source.position` on that
file, rather than looking up current Session text.

If the envelope cannot be allocated, `analyze` returns an operational failure.
Release that failure with `fail_dnit`, as for failures returned by `build`.
The current Session still owns any diagnostics the frontend produced.

## Raw product lifetime

`ast_of(result)`, `resolve_of(result)` and `sema_of(result)` return checked
`Result` borrows. Their products belong to the current serial Session view.
They do not own a retained AST or IR generation. Both the EditorSession and
Session must still exist when calling these accessors.

A successful buffer mutation, close or teardown expires the view. Starting a
new analysis, build or query operation, or replacing Session source/module/type
projections, also expires it. A checked accessor then returns an expiry error.
Pointers already obtained must not be dereferenced after that boundary.
`analysis_dnit` ends the result's own lifetime too.

The phase must include the requested product. A valid view can return a nil
product after rejected acquisition. `expr_type_of`, `decl_type_of` and
`resolved_type_of` perform the same lifetime and sema-phase checks before
reading side tables. Node IDs are meaningful only with their matching AST.
Resolve consumers must distinguish `SYMBOL_NIL` and `SYMBOL_REJECTED` before
indexing the symbol array.

`tokenize(result)` lexes the result's owned source version without mutating the
Session. The caller owns the returned token storage and frees it with
`lexer.dnit(stream, result.alloc)`. Its source bytes borrow the result, so free
the token stream before `analysis_dnit`.

## Building

`build(es, request)` runs the existing warm build engine with current overlays.
It expires previous raw analysis views. The returned `BuildOutcome` retains
its existing ownership contract. Release an error message with `fail_dnit`.
No analysis phase invokes build generators or the backend.

The editor API, CLI and manifest are supported compiler surfaces. They are
source-stable within a major version, not binary-stable. This v5 lifecycle
replaces the old pointer-only editor operations without compatibility adapters.
