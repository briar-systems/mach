# Transform allocation inventory (#3123, roadmap O1 remainder and O2)

Every allocating transform in `src/lang/me` and `src/lang/be`, its
input-consumption contract under an allocation refusal, its scratch versus
durable storage, and whether it allocates at all. A genuinely nonallocating
path is classified as such, not given an invented probe.

The apparatus is one shared allocator, `mach.lang.alloc.probe`, and one
middle-end driver, `mach.lang.me.testing`.

## The shared fail-at-N apparatus

`src/lang/alloc/probe.mach` is a counting, tracking, refusing allocator over a
backing (a page allocator by default, or one the test hands in, or nothing).
Every allocate and reallocate is one ordinal in a single sequence. It offers
`fail_at_ordinal` (refuse exactly the Nth from the arm point), `fail_from_ordinal`
(that one and every later), `make_refusing` (refuse everything, count only, for
the growth-overflow checks), `arm` (start the counted window after the fixture
is built, so a fixture allocation does not shift the ordinal a transform is
refused at), and the accounting `leaked`/`clean`/`owns`/`size_of`. `walk[C]`
runs an attempt at ordinal zero to learn the request count, then once per
ordinal to that count, checking after each that nothing is outstanding, nothing
was released twice or untracked, tracking never overflowed, and the armed
refusal fired exactly once. An operation that allocates nothing at ordinal zero
is reported `WALK_NONALLOCATING`, which is how a nonallocating path is
classified rather than probed.

`src/lang/me/testing.mach` wraps a middle-end transform as an `Attempt`: it
builds the fixture module over the probe, arms it, runs the transform through a
workspace that hands the probe straight through (`scratch.init_direct`), and
holds the outcome to the transform's promise — the refusal reported as the
allocation layer's text through the typed outcome (never a panic, never a silent
success), the input contract (unchanged, decided by `ir.equal.content_equal`
against a control built over a page allocator, or the transform's stated
remainder via a `contract` callback), and clean teardown. The harness
`transform` returns a unit outcome (`err[fail.Fail]`), so no probe helper and no
function-pointer type carries a `res[bool]` result (B-DIAG-3).

The 29 pre-existing local probe allocators (the `*_probe_allocate` /
`*_track_allocate` families across `type`, `manifest`, `obj`, `linker`,
`regalloc`, `encode`, `dwarf`, `query`, `isa`, `os`, `spirv`, `fe/resolve`,
`fe/pool`, `driver/*`, `subprocess`, `handle`, `editor`, `build/*`, `ir`,
`codegen`, and the std `allocator.testing` the middle end already used) remain
in place and green. Each is a growth-overflow refuser or a bounded tracker
whose behavior the shared probe subsumes (refuse-all, fail-at-N, fail-from-N,
tracking with size checks and poison). Consolidating all of them onto the shared
probe is a mechanical follow-up whose churn touches ~30 files and every one of
their tests; it is not attempted here because the risk to the green suite is
disproportionate to the naming win. The new middle-end walks and the backend
`ctwalk` walk are written against the shared probe, so the shared apparatus is
the one every new probe imports.

## Middle end (`src/lang/me`)

Every per-function optimizer runs its work through `me/scratch.mach`: a bounded
64 KiB arena chunk reused between functions, whose successful results never
borrow the workspace (a refused result's diagnostic is retained on the owner
before the chunk is reset). `run` owns a workspace; `run_in` takes one, so the
pipeline threads a single workspace through every pass. The durable IR is the
module (`m.alloc`); the analysis is scratch (the workspace, handed to the pass
as `alloc`); an error's text is retained on `m.alloc` before scratch is
reclaimed.

| transform | allocates | scratch | durable | input contract under refusal | walk |
|---|---|---|---|---|---|
| pipeline | yes | one shared workspace + verifier reports | module | whichever pass met the refusal; module stays releasable | `pipeline.run:refusal_at_every_module_ordinal` |
| verifier (`ir/verify` via pipeline) | yes | violation report (scratch) | none (reads only) | input unchanged | `pipeline.verify_in:refusal_at_every_ordinal` |
| mem2reg | yes | dominance/frontier/rename work | phis + dbg births on module | input, or a partial promotion with whole phi (block,value) pairs and no added block; well formed | `mem2reg.run_in:...whole_phi_pairs` |
| constfold | yes | rewrite table | folded terminators + rebuilt preds | input, or folded branches with a listed-count-preserving module | `constfold.run_in:...folded_branches` |
| algebraic | yes | rewrite table | rewritten operands | input unchanged (rewrite applied only after it is built) | `algebraic.run_in:...unchanged` |
| cse | yes | rewrite table + seen table | rewritten operands | input unchanged | `cse.run_in:...unchanged` |
| dce | yes | live/dead/work sets + reachability | erased instructions + salvaged dbg | input, or a well-formed remainder (salvage precedes compaction, so no list is left mid-erasure) | `dce.run_in:...well_formed_remainder` |
| inline | yes | recursion/counts/facts/dup + per-call remap + peel frontier | clones, metadata, peel snapshot | callers completed before it inlined; the interrupted caller keeps its call or an unreachable clone; every callee unchanged; no function lost | `inline.run_in:...callee_untouched` |
| inline (cross-module) | yes | the `body.Available` store + clone | importer clone | acquisition touches only the store, so the importer is unchanged; a pass refusal leaves the external declaration restored on detach | `inline.run_in:cross_module_body_...` |
| licm | yes | loop analysis + order | hoisted/split instructions on module | input, or a consistent partial hoist (a mid-block refusal finishes the compaction, so each instruction is listed once); well formed | `licm.run_in:...partial_hoist` |
| sroa | yes | candidate tables + block queue + slots | field slots + replacement instructions | input, or a well-formed partial split (whole rounds before the refusing round are applied); well formed | `sroa.run_in:...partial_split` |
| scalarize (`run`, `expand_gap_ops`, `expand_lane_ops`) | yes | per-function maps | replacement instructions | input, or a partial expansion with at least as many instructions and the originals present until the ending erasure | `scalarize.run_in` / `expand_gap_ops_in` / `expand_lane_ops_in:...partial_expansion` |
| vectorize | yes | loop/dependence analysis, versioning frontier | versioned clone + guards + vector ops | input, or a partial version (an unreachable clone or partly retyped fast path); the scalar loop is never removed; well formed | `vectorize.run:...partial_version` |
| `me/ir/body.extract` (inline-body product) | yes | the copy's type/function/global maps | the product module | a releasable partial product naming no more functions than the provider | `ir.body.extract:...releasable_partial_product` |

Nonallocating middle-end members, classified not probed: `vecform` (pure
queries over the module and target catalog, no allocation); the analysis
readers `me/analysis/oblivious`, and the query helpers of `constfold`,
`algebraic`, `cse` (`fold_*`, `simplify_binary`, `hash_*`, `values_equal`) which
compute over borrowed IR and allocate nothing.

## Contract violations found and fixed

1. **mem2reg / sroa candidate-capacity ordering.** The candidate arrays
   recorded their shared capacity (`cand_cap`) only after the last array was
   allocated, so a refusal of a later array freed the earlier ones at a zero
   extent (a silent under-free). Fixed by recording the capacity with the first
   array. Guard: the `mem2reg`/`sroa` walks fail at those ordinals without the
   fix.
2. **dce erasure left a list mid-compaction.** `erase_marked` salvaged
   `dbg_value`s and compacted the instruction list in one pass, so a salvage
   refusal left some entries copied and some not — a duplicated or dropped id.
   Fixed by salvaging every dropped instruction first, then compacting (the
   compaction itself allocates nothing). Guard: the dce walk's fixture places a
   live instruction between two dead ones so a mid-compaction refusal is
   observable.
3. **inline split branch had no operand capacity.** The branch the call-block
   split emits set `operand_count` but not `operand_cap`, so the module's
   teardown freed a zero-capacity operand array and leaked the branch operands.
   Fixed by setting the capacity. Guard: the inline walk reports the 48-byte
   leak without the fix.
4. **body.Available borrowed-then-owned storage.** The available-bodies store
   allocated its index tables and storage module from the module home but had no
   explicit owner, and it copied a function before growing the tables that name
   it, so a table-growth refusal stranded the copy. Fixed by giving the store an
   explicit allocator and one slot table grown before the copy. Guard: the
   cross-module walk reports the strand as a leak with the copy-before-grow
   order.
5. **vectorize replaced a builder refusal with a message.** Six sites in
   `vectorize`/reduce turned a builder allocation refusal into
   `fail.message("vectorize: preheader branch")` etc., losing the allocator's
   text and misreporting an allocation refusal as a compiler defect. Fixed by
   propagating the builder's typed error. Guard: the vectorize walk reports the
   wrong refusal text without the fix.
6. **encoder byte buffer panicked on a growth refusal.** `encode.emit_byte`
   called `panic` when its `ByteBuf` could not grow (both the capacity-overflow
   and the realloc-refusal paths). Every `emit_*` is void, so a call site could
   not meet the refusal; the panic turned an allocation refusal into a crash.
   Fixed by making the refusal sticky on the buffer (`failed` + `fail_err`) and
   rendering it as `fail.refused` at the encode boundary, after each function is
   encoded. Guard: the `ctwalk` backend walk exits with the panic without the
   fix.

## Back end (`src/lang/be`), O2 durable-versus-scratch classification

Codegen runs the backend transforms over one scratch arena
(`codegen_with_backing`'s `backing`, bound as the passes' `alloc`), reclaimed
whole after the object image is rehomed to the session allocator. The MIR module
is durable within one codegen; the object image is the durable output; a
refusal's text is interned on the session before scratch is reclaimed.

| owner | allocates | scratch | durable | error storage | existing coverage |
|---|---|---|---|---|---|
| `codegen` (whole pipeline) | yes | the arena + per-pass work | object image (rehomed to session) | diagnostic interned on session | `codegen.scratch:module_images_survive_reclaimed_working_storage` (fail-at-N over the scratch backing) |
| `lower` (ir→mir) | yes | mir builders | MIR module (scratch arena, freed after pack) | typed `res` | driven by the codegen scratch walk |
| `legalize` | yes | rewritten MIR | MIR (in place) | typed `res` | codegen scratch walk |
| `rules`/`isel` | yes | selected MIR | MIR (in place) | typed `res` | codegen scratch walk |
| `regalloc` | yes | live ranges, spill slots | MIR (in place) | typed `res`; growth-overflow probe `regalloc.add_spill_slot` | codegen scratch walk + own probe |
| `frame` | yes | prologue/epilogue MIR | MIR (in place) | typed `res` | codegen scratch walk |
| `encode` | yes | the `ByteBuf`, symbol/reloc/row marks | encoder output (scratch, packed into image) | typed `res`; the sticky `ByteBuf.failed` (fix #6) | codegen scratch walk; growth probe `encode.EncoderOutput`; encoder-owner `encoder_output_dnit` |
| `ctwalk` (N5) | yes | entry table, block-note map, monotone/entry maps, per-entry extents | none (reads the emitted stream, writes only its report) | `Refusal` detail on the walk allocator, freed by `refusal_free` | `codegen.ctwalk:refusal_at_every_ordinal_is_typed_and_leaks_nothing_x86_64` (fail-at-N over the backend for an oblivious function) |
| `obj` | yes | image sections/symbols/relocs | the `ObjectImage` | typed `res`; growth probes `obj.install_section`, tracking `obj.dnit` | own probes |
| `linker` | yes | merged sections, GOT/reloc tables | the linked image | typed `res`; growth probes + `GmTracker` | own probes |
| `dwarf` | yes | debug section buffers | debug bytes on the image | typed `res`; growth + install probes | own probes |
| `looprotate` | yes | rotated MIR | MIR (in place) | typed `res` | codegen scratch walk |
| `relaxation` (branch patch in `encode`/isa) | yes | fixup lists | patched bytes on the image | typed `res` | codegen scratch walk |
| `mir/bulk` | yes | bulk-lowering scratch | MIR | typed `res`; probe `bulk` | own probe |

Nonallocating backend members, classified not probed: `ctvalidate` and
`vecform`-style queries, `mir.opcode_name`/`desc` (table lookups), the encoding
tables and `encoding.mach` (pure), `notes.note_seeds` (writes a caller buffer),
and the `debug_input`/`unit_input` readers.

## Verification

- Full suite through the from-source HEAD compiler; new fail-at-N tests listed
  above, all green.
- `sh test/census.sh` all ok, including `real-bools` (the new `run_in`,
  `expand_gap_ops_in`, `expand_lane_ops_in` and `inline_module` production
  functions are listed; no probe helper or function-pointer type returns a bool
  result).
- Corpus layer B on the three native ELF targets, the x86_64 link leg, and the
  A/B/C fixpoint, all unchanged.
