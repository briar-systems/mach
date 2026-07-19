# Query Context and Warm Sessions

**Status**: draft for owner markup
**Scope**: the query engine's context channel, the engine's session lifecycle, and the
editor's path to driving builds
**Non-goals**: the query engine's validation/storage model (revision-based early cutoff
stays exactly as is), the pass compute logic, the build layer's records

The query database hands every derived compute one opaque context pointer (`db.ctx`).
That pointer must answer three different questions — *what services do I use*, *what
graph am I part of*, and *what build am I serving* — and conflating them in one object
is what forces build intent onto the module graph. This design separates them, and in
doing so opens the engine to sessions that outlive a single build.

---

## 1. `QueryCtx`

```mach
# the context a derived query compute reaches through `db.ctx`
#
# one record, three concerns, each with its own lifetime: the session outlives
# builds, the project and request are installed per build unit. computes cast
# `db.ctx` to *QueryCtx and read the concern they need; nothing else travels
# through the channel.
# ---
# s:   compiler services - interner, sources, diagnostics, type interner, allocator
# p:   the current unit's module graph
# req: the current unit's settled intent
pub rec QueryCtx {
    s:   *session.Session;
    p:   *project.Project;
    req: *request.BuildRequest;
}
```

The engine owns one `QueryCtx` per unit execution and installs it with `set_ctx` before
the unit's first phase. Computes that today cast `db.ctx` to `*Project` and reach
`p.s` / `p.req` cast to `*QueryCtx` and read the field that names their actual concern.
`Project.req` is deleted; the project is the graph again, nothing else.

The query engine itself does not change: `ctx` stays an opaque `ptr`, `set_ctx` stays
the installer. The record is a contract between the engine and the computes, not an
engine feature.

---

## 2. Warm sessions

A warm session is a `Session` (and its `QueryDb`) that survives across build requests.
The validation model already supports it — that is what it was built for:

- keys are stable across builds (`StableModuleId`), so cached per-module entries
  survive graph reshapes;
- inputs re-set per build with byte-equality early cutoff (`Q_FILE_TEXT` per file,
  `Q_TARGET` from the request hash, `Q_LINK_CONFIG`, `Q_MODULE_NUMBER`), so an
  unchanged input invalidates nothing;
- `invalidate(db, kind, key)` handles the one case revisions cannot see: an external
  change the session was not told about.

What is missing is an engine entry that uses an existing session instead of creating
one per unit:

```mach
# execute one unit of `bp` inside a caller-owned session
#
# the warm counterpart to `execute`: the caller's session (and its query
# database) persists across calls, so unchanged modules revalidate instead of
# recomputing. the caller owns session lifetime; the engine installs the unit's
# QueryCtx, runs the unit's phases, and leaves the cache warm.
pub fun execute_warm(bp: *plan.BuildPlan, unit: usize, s: *session.Session,
                     oa: *A.Allocator, ev: *readout.Progress, rd: *outcome.Render)
                     R.Result[BuildOutcome, Fail]
```

`execute` keeps its cold contract (fresh session per unit, torn down after) and is
reimplemented over the warm entry plus session setup/teardown. Cold CLI builds behave
identically; the warm path is new capability, not changed behavior.

Warm-session memory policy: the session allocator holds query-owned artifacts for the
session's lifetime; per-unit transients still live in a per-call arena, which the
engine opens over a private page backing (not the caller's allocator — the signature
carries none, and a caller-provided arena would never return the per-call chunks) and
tears down before returning. The channel split is carried by the session: `s.alloc`
stays the persistent channel, and a second `s.build_alloc` — identical to `s.alloc`
outside an engine call — is pointed at the per-call arena for the span of one unit
execution; the Project captures it at init, so the graph and every build-span
transient die with the call while artifact internals (which computes build through
`s.alloc` / the query DB's allocator) survive. The caller decides when the session —
and therefore the cache — dies. Codegen adapts per module on every call: the parallel
prelude probes each module's `Q_CODEGEN` (`peek_valid` — exactly `get`'s reuse test,
read-only) and fans out only the stale remainder, staging results for the pass's
adoption; a warm-valid module is served by the cache and never touched by a worker,
and a cold cache degenerates to the full fan-out.

---

## 3. The editor's front door

`EditorSession` already holds a persistent `Session` for its per-file micro-pipeline
(tokenize/parse/resolve/analyze). Whole-project operations gain a build path through
the engine:

```mach
# plan and execute a build request inside the editor's persistent session
pub fun build(es: *EditorSession, req: *request.BuildRequest) R.Result[BuildOutcome, Fail]
```

- the editor composes requests directly (it is an embedding consumer; no argv). The
  editor holds no manifest, so `build` loads it per call — one parse, on a per-call
  arena installed as the session's build channel, torn down with the plan before
  returning — and runs every planned unit through `execute_warm_all` (the engine's
  warm counterpart to `execute`'s unit loop). The build is silent: no sinks;
  diagnostics land on the session DiagList, outcome artifacts on the editor's
  allocator;
- `open`/`update` feed `Q_FILE_TEXT` through `session.load_source`, so an edited
  buffer invalidates exactly the entries that read it — but the load walk keys
  sources by module path and reads overlay-then-disk (#1588), so that alone does
  not put a buffer's text into a build. `open`/`update` therefore also mirror the
  buffer into the session's source overlay under the buffer's key: a buffer
  registered under its module's on-disk path builds with its unsaved text, while a
  non-path key (an LSP URI) never matches a module and is inert to builds;
- repeated builds with an unchanged request and one edited file recompute only that
  file's parse and whatever its exports firewall lets through — the incremental
  behavior the query catalog documents, finally exercised by a consumer that holds
  the cache warm.

The editor's existing per-file API is untouched; `build` is additive surface.

---

## 4. Cache-key granularity

With warm sessions real, the all-fields request hash keying `Q_TARGET` becomes
measurable: a request differing only in a field no query reads (`jobs`) currently
re-resolves the world. The refinement, applied only if a warm workload shows it
matters: the hash function takes the field subset a query family actually consumes —
`request.semantic_hash` (drops scheduling/emission fields) alongside the full
`request_hash` (the build identity). Two functions, each with a stated contract, both
serialized through the same `encoding.binary` writer so a new field must be placed in
one of them deliberately.

This stage ships last and only with measurements from a real warm consumer.

---

## 5. Adoption

| # | Stage | Builds |
|---|---|---|
| 1 | QueryCtx | the record, engine installation, compute casts; `Project.req` deleted |
| 2 | Warm entry | `execute_warm`; `execute` reimplemented over it; cold behavior byte-identical |
| 3 | Editor build | request composition + `build` on `EditorSession`; a warm end-to-end test (edit one file, rebuild, assert recompute scope via query revisions) |
| 4 | Key granularity | `semantic_hash`, only with stage-3 measurements justifying it |

Gates throughout: full suite, int leg, self-build fixpoint and timing; stage 2 must
show zero behavioral delta for cold builds; stage 3 adds the first warm-session
regression tests. The mach-lsp pin sees only additive API until it advances.

---

## `fin` and ret-position evaluation

A `ret` expression is fully evaluated before the pending `fin` bodies run: with an
active teardown `fin` (the warm engine's per-call arena, the editor build's
build-span arena), `ret f(...)` runs the callee over live state, then the fins,
then returns the bound value. Binding the result to a local and returning the
local is equivalent; the engine and editor sites keep the explicit bind for
readability.
