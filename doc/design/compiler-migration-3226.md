# Compiler migration to std 2.0 (#3226, lane C1)

C1 is the first compiler-side lane of the v5 migration. It bootstraps CI
through the v5 stage, pins `dep/std` on std 2.0, migrates the compiler's shared
outcome and allocation adapters to their final forms, and fixes the producer
and consumer types the consumer lanes C2, C3 and C4 migrate against. The
language contract is `tagged-values.md`; std's frozen signatures are its
`MIGRATION.md` (std dev `e204cb81f`); the pin that built this lane is
`migration-compiler-3218.md`. C5 removes what this document marks as owed.

## Pins

| | |
| --- | --- |
| bootstrap chain | `.github/actions/setup-mach/bootstrap.py`: published 4.26.5, bridge `878a8f66` single, audited `b65afb97` fixpoint, v5 `2a2918b23` fixpoint. The v5 stage's std is `168a9f760` (std 1.0.1) because a stage's std is what its own 1.x compiler source builds against, not what the tree it produces compiles |
| `dep/std` | std dev `e204cb81f39fb31ed3bb33802ce9f2331626a640` (S1 to S6 and S8 merged, every S1 to S4 signature frozen), declared as `commit/` so the build verifies the gitlink |
| compiler that builds the tree | the v5 stage's fixpoint compiler (2a2918b23 with std 1.0.1); the 4.30.0 seed cannot read std 2.0 |

`mach dep pull` is not run by hand: it restores the committed gitlink, which is
already the pin. `mach build` skips `test` blocks; only `mach test` compiles
them.

## The reviewed type set

The representation rules are std's ("Representation rules" in its
`MIGRATION.md`), applied to the compiler's own outcomes.

| compiler outcome | final representation | rule |
| --- | --- | --- |
| a phase or query that can fail after diagnostics were recorded | `res[T, fail.Fail]`, `err[fail.Fail]` | `tag Fail: u8 { reported; message: str; }`: `reported` says the diagnostic is already on the session and nothing further is said; `message` is an internal failure whose text is preserved. `reported` is first so a zero outcome invents no text. Exactly two cases, so an exiting `sel r.err.reported` chain guards `message` for the rest of the block |
| a driver operation (plan, build, publish, dependency, init, test run) | `res[T, outcome.Fail]`, `err[outcome.Fail]` | `tag Fail: u8 { reported; user: str; internal: str; environment: str; }`; the exit code derives from the case (1, 2, 3). A reported failure has no text |
| a phase's standing | `fail.PhaseStatus { kind: PhaseKind; rejections: u64; }` with `tag PhaseKind: u8 { accepted; rejected; internal: str; }` | the internal text lives on its case, never as an empty string beside a kind |
| an allocation refusal met by the compiler | `fail.refused(e)` = `Fail.message{alloc.text(e)}`, `outcome.refused(e)` = `Fail.internal{alloc.text(e)}` | the compiler recovers from no refusal at the site that met it; it reports and stops. An allocation refusal stays an internal failure (exit 2) as today; reclassifying it as environmental changes exit codes and is C4's decision to make explicitly |
| a closed-catalog fault | `tag fail.Catalog: u8 { malformed: Member; unsupported: Unsupported; internal: Member; }` | the three classes have different payloads; `malformed_member`, `malformed_member_in`, `unsupported_member`, `unknown_member`, `unknown_member_in` construct them. `catalog_text` answers `res[str, format.FormatError]` (the only failure a literal format meets is the allocator's, carried as the format's `alloc` case); `catalog_interned` answers `res[str, Fail]` |
| what a build recorded | `tag outcome.BuildEvent: u8 { unit: BuildUnitEvent; note: u32; fail: Fail; diagnostics: DiagnosticBatch; }` | a kind beside a raw union was a hand-rolled tag; every payload is owned by the outcome's allocator and released by `outcome_dnit` |
| an outcome's `record_*` and `merge` | `err[allocator.Error]` | the only failure is the allocator's; a refused copy or push leaves the outcome exactly as it was and releases the copies made before it. `record_diagnostics` is `err[outcome.Fail]` because it composes two subsystems' snapshots (whose refusals it reports as internal) with its own push |
| a message result, `R.Result[T, str]` | `res[T, E]` with a closed domain `E`, never `str` | the error is a value the caller acts on; text is presentation (`fail.describe`, `outcome.describe`, `fail.status_text`) rendered at the CLI, never matched |
| an error-or-nothing, `O.Option[str]` | `err[E]` | unit success with a typed refusal |
| genuine absence, `O.Option[T]` | `opt[T]` | empty, no match, no element, unset |
| unit success, `R.Void` with `ok_void`/`void_of` | `err[E]` | the unit type disappears with the carrier |
| a real yes/no | `bool` | unchanged (`changed`, `found`, `committed`); the failure-kind census keeps its allowlist |
| a closed integer enumeration whose members carry no payload | unchanged integer kind (`ExprKind`, `TokenKind`, `BuildSeverity`, `ArtifactKind`, `subprocess.Request`, ...) | not an outcome; `BuildSeverity` is ordered and compared with `>`. Converting the compiler's kind catalogs is outside #3226 |
| a compiler-implemented allocator backend | `fn_allocate: fun(ptr, usize, usize) opt[ptr]` | the callback contract is std's; the twenty-nine local probe allocators (`*Probe` records with `fail_at`, `live`, `calls`) answer it directly |

### What stays a raw union

A `uni` whose alternatives really overlap the same storage stays a `uni`:
`float` bit views, `Value`/instruction payload unions in the IR that are read
under a discriminator the IR owns and serializes, register and immediate
operand views in the instruction selectors, and the binary object format
records. The test is the one in `tagged-values.md`: a tag is a value with one
selected case and a discriminator at offset zero; a raw union is
representation. `BuildEvent.data`, `TemplateError` and the two `Fail`
records were kind-beside-union or kind-beside-message and are tags now; the
rest of the compiler's unions are representation and C3 keeps them.

### Predicates and presentation

`sel` takes a place, never a call. Where a consumer tests a call operand
(`R.unwrap_err(...)`, `outcome.from_fail(...)`), the adapters provide
predicates: `fail.is_reported`, `fail.is_message`, `fail.is_accepted`,
`fail.is_rejected`, `fail.is_internal` (on a status), `outcome.is_reported`,
`is_user`, `is_internal`, `is_environment`, `alloc.refused` (on
`err[allocator.Error]`). `fail.text` and `outcome.text` answer `opt[str]`;
`describe` answers one line of text for every case (`fail.REPORTED_TEXT` for
a reported failure, never nil); `outcome.with_text` replaces the text keeping
the case; `outcome.release_text` releases the text a failure owns. These are
the final API, not shims.

## The allocation layer

`mach.lang.alloc` is the compiler's allocation layer: `Allocator`, `Error`,
`allocate`, `zallocate`, `reallocate`, `deallocate` forwarded from
`std.allocator` unchanged (`res[*T, Error]`, `err[Error]`, the frozen
contract), `text(e)` rendering the refusal once (`exhausted` is the "out of
memory" the compiler has always reported), and `refused(r)`. A migrated
site allocates through it and lifts a refusal with `fail.refused` or
`outcome.refused`. It owns no tracking allocator: the twenty-nine local probe
allocators in tests are candidates for `std.allocator.testing` where its
4096-entry table suffices, a per-lane cleanup.

## The translation shim

Every unmigrated site compiles against std 2.0 through `mach.lang.legacy`,
one module per std module whose signatures moved (`allocator`, `page`,
`arena`, `fixed`, `testing`, `vector`, `map`, `string`, `text`, `path`,
`format`, `print`, `writer`, `filesystem`, `transaction`, `toml`, `binary`,
`env`, `exec`, `parse`, `time`; 21 modules, 158 wrappers, 225 forwards). A
façade forwards every unchanged member and wraps every changed one in its
1.x carrier, spelling the typed refusal through one renderer per domain in
`mach.lang.legacy` (`alloc_text`, `io_text`, `fs_text`, `str_text`,
`format_text`, `toml_text`, `encode_text`, `decode_text`, `env_text`,
`exec_text`, `parse_text`, `thread_text`, `read_text`, `write_text`). A
consumer changes only its `use` line, so its sites compile unchanged; that
is the whole of the shim's footprint, and `grep '^use .*mach\.lang\.legacy'`
counts what remains.

The shape at a subsystem boundary, in both directions:

- A migrated module consuming an unmigrated neighbour reads the legacy
  carrier directly (`fail.catalog_interned` reads `intern.intern`'s
  `R.Result[StrId, str]` and lifts the text into `Fail.message`;
  `outcome.record_diagnostics` reads the two `snapshot_from` results the same
  way). It does not add a façade for the neighbour and does not classify the
  neighbour's text.
- An unmigrated module consuming a migrated adapter spells the typed outcome
  as its own legacy carrier at the call site through the domain renderer
  (`me.ir.verify.catalog_named` turns `res[str, FormatError]` into
  `R.Result[str, str]` with `legacy.format_text`; the build engine turns a
  `record_*` refusal into `R.err(alloc.text(e))`; `fail.lift` turns an
  `R.Result[T, str]` into `R.Result[T, Fail]`). The site is removed when the
  module's own return types migrate.
- A callback contract that changed (writer sinks, allocator backends, the
  transaction's producer) is implemented in the new shape directly; where a
  legacy producer must still be handed to a new contract the façade adapts it
  (`legacy.transaction.prepare` wraps a `R.Result[R.Void, str]` producer in a
  `Producer[W]` whose `err[WriteError]` reports `stalled{0}`, which the
  transaction folds into `REJECTED` exactly as 1.x did).

What could not be shimmed and was migrated here: the cancellation scope's
reason is a tag, so `mach.subprocess` owns `Request` (the closed kind the
validation oracle serializes, with the scope's 1.x codes) and `request_of`
maps `cancel.Reason` onto it; the exit status is `os.ProcessStatus`, so the
supervisor observes it through `exec.exited`/`signaled`/`code`; `os.read_dir`
and `dirent64` are gone, so the three directory scans (`driver.union`,
`driver.config`, `build.cache.store`) use `os.DirectoryCursor`; the event
source's `make`, `close` and `next` report typed outcomes; the thirteen
writer sinks return `res[usize, WriteError]`; the allocator backends answer
`opt[ptr]`.

## Partition for C2, C3 and C4

Counted on this lane's head with `grep -c 'R\.Result\['`, `'O\.Option\['` and
`'\b(ok_void|void_of)\b'` per file, plus the façade imports each lane
removes. C1's own 119 `R.Result[` sites are the façades and `fail.lift`, the
shim itself.

| lane | subsystems | files | with legacy sites | `R.Result[` | `O.Option[` | `ok_void`/`void_of` | façade imports (files) |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| C2 | `fe/*` (lexer, parser, resolve, sema, comptime, ast, doclint, pool, path), `source`, `session`, `query`, `editor`, `intern`, `type`, `diagnostic`, `module`, `ct/*`, `deprecation`, `embed`, `float` | 51 | 36 | 2,851 | 908 | 374 | 146 (32) |
| C3 | `me/*` (ir, lower, pass, transform, analysis), `be/*` (codegen, mir, linker, obj), `target/*` (isa, abi, os, of), `target`, `layout` | 136 | 111 | 10,212 | 1,075 | 1,182 | 314 (90) |
| C4 | `cli/*`, `bin`, `driver/*`, `driver`, `build/*` (engine, plan, request, emit, fingerprint, cache, testing), `manifest`, `publication/*`, `subprocess`, `validation/*`, `readout`, `textbuild`, `fuzz/*`, `handle`, `version` | 59 | 57 | 5,409 | 741 | 434 | 382 (55) |
| total | | 246 | 204 | 18,472 | 2,724 | 1,990 | 842 (177) |

Each lane starts from this lane's reviewed commit, migrates its files to the
type set above, removes its façade imports as each file's own return types
migrate, and leaves `mach.lang.legacy` for C5 to delete when
`grep -rl 'mach\.lang\.legacy' src` is empty. The lanes do not add
compatibility fallbacks and do not classify a neighbour's rendered text.

## C4 decisions

An allocation refusal met by the driver stays an internal failure:
`outcome.refused(e)` is `Fail.internal{alloc.text(e)}` and the process exits
2, exactly as before the migration. The reason is where the refusal is met.
The driver's allocations are the compiler's own working set (module tables,
query storage, plan and outcome records), sized by the input and the
compiler's choices, not by the machine: a refusal there says the compiler
asked for more than it should have, which is the internal class, and a
script that keys on exit 3 to retry or to report the host rather than the
compiler must not be told that. Environmental failures (exit 3) stay what
they are today: the host refusing a filesystem, process or lock operation
the compiler was entitled to make. The cost of the alternative would have
been an exit-code change visible to every caller for no diagnostic gain, since
the rendered text ("out of memory") is the same either way. Pinned by
`mach.cli.diagnostic.render_fail:an_allocation_refusal_met_by_the_driver_exits_internal`
(the case, the rendered text and the code through both `render_fail_w` and
`outcome_code`) alongside C1's `mach.lang.build.outcome.fail:cases_text_and_lift`.

Zero-valued carriers: `err[E]` and `res[T, E]` are zero as their `err` case
(std's canonical layout puts the failure first), where the legacy
`O.Option[str]` was zero as "no error". Every accumulator the driver keeps
across a sequence of clean-up steps (`directory_close`, `operation_close` in
`cli.cmd.init`) is therefore initialised to `.ok{}` explicitly, and the lane
carries no other zero-relying carrier.

## Verification

- unit suite, from-source compiler built by the v5 stage, debug and release:
  2965 passed, 0 failed (2959 at `origin/dev` 0e4c62b5c, dev merged through
  PR #3282; by name +3 `mach.lang.alloc`, +2 `mach.lang.fail`, +1
  `mach.lang.build.outcome.fail`, no test removed; the out-of-catalog
  `FailKind` assertion inside `mach.cli.diagnostic.render_fail_w` went with
  the kind)
- `sh test/census.sh` all ok
- corpus layer B, x86_64-linux and spirv: unchanged goldens
- link leg x86_64-linux
- corpus layer B: 196 pass, 0 fail, 8 skip (the declared spirv limitations)
- link leg: 140 pass, 0 fail, 0 skip
- three-generation fixpoint from the v5 stage: A by the stage, B by A, C by
  B, A = B = C `38861714` (no code generation changed)
- cross-builds of the compiler for darwin-aarch64, darwin-x86_64 and
  windows-x86_64

## C4 verification

Counted over the 63 files of the C4 partition (the 59 above plus
`build/outcome`, `fail`'s C4 hunk, `driver/tests` and `cli/diagnostic`),
before at 2f2f47cb6 and after on this lane's head:

| pattern | before | after |
| --- | ---: | ---: |
| `R.Result[` | 6,329 | 94 |
| `O.Option[` | 843 | 53 |
| `ok_void` / `void_of` | 522 | 8 |
| `mach.lang.legacy` imports | 411 | 0 |
| `R.Result[T, str]`-shaped | 4,785 | 78 |

The 78 `R.Result[T, str]` sites left are the lane's own translation shims
(the legacy `<name>` over `<name>_typed` in `session`, `query`, `manifest`,
`publication`, `publication/plan`, `publication/testing`, `driver`,
`driver/registry`, `handle`; the legacy-kept callbacks
`load.resolve_module_member_const_load_cb`, `passes.read_definition_cb`,
`passes.prepared_surface_recipe_cb`, `editor.buffer_definition_cb`,
`registry`/`fuzz` `provide_debug_descriptor`) and the reads of the
neighbours' legacy-shaped exports that C2 and C3b kept under their canonical
names (`source.get`/`add`/`prepare_load`/`prepare_release`/`copy_file`/
`snapshot_from`/`line_bounds`, `diagnostic.gate_error`/`error`/`get`/
`snapshot_from`/`replay_into`/`content_equal`/`truncate`/`builder_init`/
`attach_fix`/`commit`/`last_id`, `parser.parse`, `comptime.eval*`, `embed.*`,
`linker.link`/`link_images`/`link_images_captured`, the assembly vtable's
`ct_scan`, `dwarf.debug_descriptor`). None is a C4 return type.

- unit suite through the from-source compiler (A by the v5 stage): 2997
  passed, 0 failed (2996 at `origin/dev` 3b66e0f70; by name +1
  `mach.cli.diagnostic.render_fail:an_allocation_refusal_met_by_the_driver_exits_internal`,
  no test removed)
- `sh test/census.sh` all ok (real-bools: the seven `reusable` predicates that
  moved from `R.Result[bool, fail.Fail]` to `res[bool, fail.Fail]` listed)
- corpus layer B x86_64-linux: 102 pass, 0 fail, 0 skip; spirv: 94 pass, 0
  fail, 8 skip; goldens unchanged
- link leg x86_64-linux: 140 pass, 0 fail, 0 skip
- three-generation fixpoint from the v5 stage: A = B = C `4ce07b110c63bc3c`
- cross-builds of the compiler for darwin-aarch64, darwin-x86_64 and
  windows-x86_64 (the darwin and windows blocks of `build/cache/compiler` and
  the PATHEXT read in `cli/util` are only compiled there)

## C5a: the compiler-side unshim

The four modules no lane owned (`embed`, `source`, `diagnostic`, `ct/probe`)
read std 2.0 directly and answer the type set: `source.add`/`update`/
`prepare_load`/`load`/`copy_file`/`snapshot_from` are `res[T, fail.Fail]`,
`prepare_release` is `err[fail.Fail]`, `get`/`line_start`/`line_bounds` are
`opt[T]`; `diagnostic.get`, `resolve` and `last_id` are `opt[T]` (an index
past the store, a stale id and an empty store are absences, exactly as
`vector.get` answers them), every append and builder step is `res[T,
fail.Fail]` or `err[fail.Fail]`; `embed.get` and `path_arg_span` are `opt[T]`,
`resolve_arg` keeps `manifest.TemplateError`, `escapes_root` and `refresh`
answer `fail.Fail`. `src/lang/legacy.mach` and `src/lang/legacy/` are deleted
with their census exclusion.

Every joint contract is flipped in one change: the comptime evaluation
carrier and the `PhaseCapabilities[T]` callbacks are `res[T, EvalFail]` over
fe/comptime, fe/sema, me/lower and driver/load; `parser.parse` answers
`res[fail.PhaseStatus, state.ParseFail]`; `DefinitionReader.read` is
`fun(ptr, ModuleId, DefinitionPhase) res[Definition, fail.Fail]`;
`fields.capture`'s prepare callback is `fun(*T, TypeId) err[fail.Fail]` and
the capture answers `res[Graph, fail.Fail]`; every codegen-boundary vtable
typedef in `target/isa.mach`, `target/of.mach`, `be/codegen/rules.mach` and
`be/codegen/encode.mach` (`SelectFn`, `EncodeFn`, `EmitAsmFn`, `AsmCtScanFn`,
`EmitModuleFn`, `RelocTraitsFn`, `NormalizeImageFn`, the attribute trio, the
writer, parser and image quintet, `DebugProduceFn`, `ExpandFn`,
`EncodeFunctionFn`, `PatchBranchFn`, `target.DebugDescriptorProvider`) is
typed and every ISA and OF implementer registers its typed name. `EvalFail`
and `ParseFail` keep their kind-beside-message records under the flipped
carrier: their kinds are closed integer catalogs that the evaluator and the
parser compare, and the design leaves kind-catalog conversion outside #3226.

The 87 `<name>_typed` cores took back their canonical names and their 1.x
exports are gone; C3b's named twins keep the names C3b chose
(`codegen_unit`, `prepare_debug`, `image_init`, `section_install`,
`symbol_add`, `rehome`, `emit_image`, `lower_module`, `validate`, `walk`,
`produce_debug`, `select_function`, `emit_instr`, `check_operand_banks`,
`encode_module`, `encode_module_asm`) and their legacy exports are deleted;
the 57 `<name>_legacy` vtable wrappers, the four identity `_cb` callbacks,
`sema.legacy_sema_result`/`legacy_unit`, `fields.captured_of` and
`handle.chunk_get`/`chunk_edit`/`chunk_reserve`/`chunk_push` (the type table
reads `chunk_at`, `chunk_editor`, `chunk_grow` and `chunk_append` in
`handle.Error`) are deleted. `fail.lift`, `lift_res`, `lift_err`, `lift_of`,
`lift_opt`, `lower`, `lower_res`, `lower_err`, `lower_of`, `lower_unit`,
`lower_unit_of`, `lower_opt`, `discard`, `discard_refused` and
`outcome.lift_user`/`lift_internal`/`lift_environment` (with their `_unit`
forms), `lower`, `lower_unit`, `lower_fail`, `lower_fail_unit` are deleted
with their last caller; a unit outcome of a result whose value is not needed
is spelled at the site (`if (sel r.err) { ret err[Fail].err{r.err}; }`).

A pattern the flip surfaced: a `fail.Fail` that was lowered to text and lifted
back (`fail.message(fail.describe(f))`) lost its `reported` case; every such
site now propagates `f`. `type.mach`'s absent table slot is one constant
(`TABLE_INDEX_TEXT`), a defect, never an input fault.

### C5a verification

Counted over `src`, `test/cases`, `test/link/cases` and the corpus fixtures on
this lane's head (before at 2ac0cc720):

| pattern | before | after |
| --- | ---: | ---: |
| `R.Result[` | 1,498 | 0 |
| `O.Option[` | 515 | 0 |
| `ok_void` / `void_of` | 89 | 0 |
| `R.Void` | 762 | 0 |
| `use R: std.types.result` / `use O: std.types.option` | 186 | 0 |
| `mach.lang.legacy` imports | 4 files (plus the façade itself) | 0 |
| `<name>_typed` twins / `<name>_legacy` wrappers | 87 / 57 | 0 / 0 |

The two `R.Result[` spellings left under `test/fuzz/corpus/{lexer,parser}`
are retained fuzz inputs (bytes the lexer and parser are handed), not compiler
code, and stay as retained.

- unit suite through the from-source compiler (A by the v5 stage): 2997
  passed, 0 failed (2997 at `origin/dev` 2ac0cc720 built the same way;
  per-module counts identical; by name the only delta is the 28 test labels
  under `mach.cli.cmd.{build,clean,dep,doc,info,init,run,testing}` whose
  function under test lost its `_typed` suffix, no test added or removed)
- `sh test/census.sh` all ok (real-bools 122 listed: the two deleted `_typed`
  twins delisted, nothing else; a predicate answering `res[bool, Fail]` stays
  listed)
- corpus layer B: x86_64-linux 102 pass, 0 fail, 0 skip; aarch64-linux 102
  pass, 0 fail, 0 skip; riscv64-linux 102 pass, 0 fail, 0 skip; spirv 94
  pass, 0 fail, 8 skip; goldens unchanged
- link leg x86_64-linux: 140 pass, 0 fail, 0 skip
- three-generation fixpoint from the v5 stage: A = B = C `947240c3`
- cross-builds of the compiler for darwin-aarch64, darwin-x86_64 and
  windows-x86_64 with A
- `mach init` of a scratch project under `out/` and a build and run of it

## Owed to C5

- ~~delete `src/lang/legacy.mach` and `src/lang/legacy/` once no import remains~~ done (C5a)
- ~~delete `fail.lift`~~ done (C5a), with every `lift*`/`lower*`/`discard*` shim
- `std.types.result` and `std.types.option` leave std now that the compiler
  has no `R.`/`O.` use (the std lane, after C5a lands)
