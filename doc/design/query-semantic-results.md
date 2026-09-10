# Semantic results: the query contract for language products

Status: accepted with #3220 and #2999. Binding for every product the
language work adds to the query database (tag layouts, case guards,
canonical tag tables, or any later semantic result).

## What a product declares

A product is one query kind registered once per session through the
registration surface in `src/lang/query.mach`:

- `register_input` for bytes the host sets directly (`set_input`). An
  input carries its own revision and no dependencies.
- `register_derived` for a product whose bytes the compute callback
  builds and the database owns. Equal bytes on recomputation keep the
  previous revision.
- `register_derived_owned` when the bytes are a handle to storage the
  product allocated. The finalizer releases that storage whenever the
  database drops the bytes: replacement, failed publication, retirement
  and teardown. An owned product never aliases a previous value; if the
  new value equals the old one the candidate is released through the
  finalizer and the published entry keeps its identity and revision.
- `register_derived_equatable` for owned products with a structural
  comparator; use it when byte identity would make every recomputation
  look changed (IR modules are the existing example).
- `register_metadata` for a revision the database cannot store: an
  external file, a manifest overlay, a tool version. The provider reads
  `(present, revision)` on demand and is sampled once per operation.

Every product therefore declares three things: how its inputs are
observed (below), which revision it carries (the database stamps it on
publication), and who owns its bytes (the database, through the
finalizer when one is registered).

## How a product joins invalidation

Dependencies are recorded by reading, never by hand. Inside its compute
callback a product calls `get` (or `depend`) for every query it reads
and `observe` for every metadata revision it consults. The database
records the edge on the computing owner together with the dependency's
revision and diagnostics revision. A validation pass never records
edges on the query it is validating for.

Validation is transitive. Before a cached product is reused, each
dependency is itself validated or recomputed, depth first, inside the
current operation. A dependency that recomputes to an equal value keeps
its revision and stops propagation (early cutoff). A dependency whose
value or diagnostics changed advances its revision, so every product
above it recomputes. A dependency that fails makes the requesting
product fail; the previous value is never served. Cycles are detected
on the computing stack before any recursive validation.

Diagnostics are part of the product. Each derived product owns the
diagnostic store its compute wrote into. Equal bytes with different
diagnostics still advance the diagnostics revision, and dependents
observe it. Presentation (`collect`) replays diagnostics in dependency
order once per origin, cold or warm.

A new product kind therefore joins invalidation by doing only this:
pick a registration that states ownership, read every input through
`get`/`observe`, write diagnostics only to the store the callback
receives, and return bytes the finalizer can release. Nothing else in
the engine needs to change, and no kind may cache across operations
outside the database.

## Operations and consumers

All reads happen inside an operation (`begin` ... `end`). `end` verifies
that every sampled metadata revision is unchanged and refuses success
otherwise. Inputs cannot change and products cannot be retired while an
operation or its uncollected presentation is live. The build, check and
editor consumers share the same revision-stamped products; a consumer
identifies a result by its source revision and the selected target
input (`Q_TARGET`), which is an ordinary input and invalidates every
product that read it.

## What the editor lifecycle guarantees

- `open`/`update` publish the buffer text as the `Q_FILE_TEXT` input and
  the session overlay atomically; a failed preparation leaves every
  buffer, overlay and view as it was.
- `analyze` runs one phase and returns an owned snapshot: diagnostics,
  the source versions they cite, the selected target and configuration.
  The snapshot outlives the session. Raw products (`ast_of`,
  `resolve_of`, `sema_of`) borrow a serial view that expires on the next
  mutation, target change, close or query use, and report expiry
  instead of dangling.
- `close` retires the buffer through `prepare_retirement` /
  `commit_retirement`: dependents are freed before their inputs, the
  overlay and source payload are released, the `FileId` survives and is
  reused by the next open of the same path, and unrelated buffers,
  their inputs and their products keep their revisions. A close during
  an outstanding operation or presentation is refused and changes
  nothing.
- A semantic product added later inherits all of this: it is retired
  with the buffers it depends on, presented with the diagnostics it
  wrote, and never reused across a target or source revision it did not
  observe.
