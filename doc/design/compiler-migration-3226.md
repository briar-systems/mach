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

## #3112 removal inventory

C5b (`feat/3226-c5b`, on dev `33d60001e`). Every item of #3226's "Exact
removal inventory" that C5 owns, with the commit that retires it, the
diagnostic a user still writing the form receives, and the negative test that
holds it (each test's source carries an `intentional removal test` comment).
The last row is the item C5 does not own.

| # | item | commit | diagnostic (located where the form is written) | test |
| --- | --- | --- | --- | --- |
| 1 | `:^` and `:^T` refused, `:>T` only | `c6bf473ef` | `` `:^` and `:^T` were removed in 5.0.0; declassification is `expr:>T` and always names its public result type, as in `x:>u32` `` at the operator (parser, `parse_removed_strip`); dep/std at `e204cb81f` carries no `:^` (only `::^T` casts) | `mach.lang.fe.parser.expr.parse_strip:removed_caret_forms_are_refused`, `mach.lang.driver:colon_caret_is_refused_with_the_migration_diagnostic`; the generic-body replacement for the untyped form: `mach.lang.driver:generic_strip_target_is_checked_per_instance` |
| 2 | alias dependency keys, nested `dep/<id>/dep/` realizations, root `mach.lock` | `09f7cf5da` | `[dep.foo] realizes project 'std'; alias keys were removed in 5.0.0: ... rename the table to [dep.std] and the directory to dep/std` (pull, verify and the build; F3 had only the pull note and no build check); `dependency 'a': dep/a/dep/b is a nested realization; nested realizations were removed in 5.0.0 (...): delete dep/a/dep` (an empty gitlink directory passes); `mach.lock was removed in 5.0.0 and is refused; the committed gitlinks under dep/ are the pins: delete mach.lock` (every command that opens the project) | `mach.cli.cmd.dep.pull:an_alias_key_is_refused_naming_the_removal`, `mach.lang.driver.deps.realize_git_submodule:alias_key_is_refused_after_checkout`, `mach.lang.driver:dep_closure_refuses_an_alias_key_before_any_conflict`, `mach.cli.cmd.dep.verify:a_nested_realization_is_refused_and_an_empty_gitlink_directory_is_not`, `mach.cli.cmd.dep.pull_and_verify:a_stale_lock_file_is_refused_naming_the_removal` |
| 3 | `sysv` as an alias of `sysv64` | `c7b4e6069` | `` `$mach.abi.sysv` was removed in 5.0.0; the registry spells this ABI `sysv64`: write `$mach.abi.sysv64` `` at the path (the manifest never accepted the alias) | `mach.lang.fe.comptime.tag_for:removed_sysv_alias_is_refused_by_name` |
| 4 | ambiguous target/profile/artifact selection without an explicit default | `f63b673d2` | `mach.toml: several {targets are declared, none matches the host \| profiles are declared \| artifacts support the selected target} and none is marked `default = true`; no ... is selected by table order: mark exactly one [...] with `default = true` or select one with {--target \| --profile \| --bin/--lib}` (F3 had left the warned first-declared fallbacks) | `mach.lang.manifest.resolve_target:native_without_a_host_match_uses_the_sole_or_default_declaration_never_table_order`, `mach.lang.manifest.resolve_profile:several_profiles_without_a_default_are_refused_naming_the_default_key`, `mach.lang.build.plan.plan:a_test_goal_without_a_default_among_several_artifacts_is_refused_naming_the_default_key`, `mach.lang.driver:a_test_build_over_several_artifacts_without_a_default_is_refused_at_planning`, `mach.cli.cmd.doc.build_project:several_artifacts_without_a_default_are_refused` |
| 5 | embedded paths escaping the project root | `467631b08`, `b1cca76f1` | `` `embed` path escapes the project root; an embedded file must live inside the project (4.30 read it with a warning, 5.0.0 refuses it and does not read the file) `` at the decorator; the driver skips the path when collecting embed inputs so the file is never opened; root and file are compared in absolute coordinates (a relative root used to flag every embed) | `mach.lang.driver:embed_outside_the_project_root_is_refused_and_never_read`, `mach.lang.embed.containment:relative_roots_and_source_paths_share_absolute_coordinates` |
| 6 | `[project] name`, `description`, `mach`; `[profile.*] emit_ir`, `emit_asm` | `08c080a27` | `mach.toml: [project] key 'name' was removed in 5.0.0; it was accepted and never read: remove the key` (root and dependency manifests, `dep '<id>': ` prefixed); the deprecated-key record and driver warnings are deleted | `mach.lang.manifest.parse:the_five_unread_keys_are_refused_naming_the_removal_in_both_forms`, `mach.lang.manifest.parse:mach_key_is_refused_as_removed_not_unknown`, `mach.lang.manifest.native_owner:removed_key_rejection_owns_only_its_message`, `mach.lang.driver:removed_manifest_keys_are_refused_for_the_root_and_each_dependency_naming_the_removal` |
| 7 | `$project.name`, `$project.description` (#3128) | `53f683f8e` | `` `$project.name` was removed in 5.0.0 with the `[project] name` manifest key; the project is identified by `$project.id` `` and `` `$project.description` was removed in 5.0.0 with the `[project] description` manifest key; there is no comptime project description `` at the path; not re-sourced (the config, build context and fingerprint fields are deleted); a rejected rooted path now reports its own message instead of the generic bare-ident error | `mach.lang.fe.comptime.resolve_project_path:removed_name_and_description_are_refused_by_name`, `mach.lang.driver:project_name_and_description_paths_are_refused_naming_the_removal` |
| 8 | MOS 6502 target | `f7f0c644a` | `target 'mos6502' was withdrawn and removed in 5.0.0; no isa or abi implementation is registered for it: retarget the [target.*] table to a supported tuple` at target resolution (manifest `isa`/`abi`); `$mach.arch.mos6502` is an unknown tag. Deleted: `src/lang/target/isa/mos6502/` (4 files), `isa/mos6502.mach`, `abi/mos6502.mach`, the registry rows, `freestanding` OS row, `TUPLE_TARGET_UNAVAILABLE`, `test/golden/mos6502/`, its `engines.conf` row and SKIPS, `test/fuzz/corpus/asm/mos6502-in-any.asm`; `da65` was never pinned in `tools.lock`. Arch catalog version 2 (`arch.MOS6502_WITHDRAWN = 4` reserved, no row). `closed-catalogs-3124.md` records the deleted `mos6502.Opcode` catalog. The width legalization pass is untouched: `mach.lang.be.codegen.legalize` 30 tests pass and the riscv32 column still refuses every case through it (`* ab` declared skip, 204 cells) | `mach.lang.driver:mos6502_target_is_refused_as_withdrawn`; `mach.lang.target.isa.arch_id_for:name_roundtrip` (`mos6502` is `ARCH_UNKNOWN`); `mach.lang.fe.comptime.tag_for:registry_names_are_the_only_names`; 18 mos6502 unit tests deleted with their sources (listed under Verification) |
| 9 | explicitly defaulted library public entry, no implicit `lib.mach` | `4a4d8cf91` | `` project 'x' declares no artifact, so it has no public module; the implicit `lib.mach` entry of an artifact-less dependency was removed in 5.0.0: import a full path, or declare a static or shared [artifact.*] table marked default = true in its manifest `` at the `use`; `mach init --lib` scaffolds its artifact `default = true` (`1b725eaf4`) | `mach.lang.driver:bare_import_of_an_artifact_less_dependency_names_the_removed_lib_mach_fallback` |
| 10 | doc/, `mach init` templates and fixtures | `1b725eaf4` and each row's commit | `doc/manifest.md`, `doc/cli.md`, `doc/language/{grammar,secrecy,comptime,comptime-mach,modules,decorators}.md` and the mach skill describe only the retained forms and name each refusal; `doc/design/release-shape.md` and the closed historical design records keep their "removed in 5.0.0" statements; the 4.30.0 CHANGELOG section is history | the negative tests above; `grep` of `:^`, `emit_ir`, `$project.name`, `$mach.abi.sysv`, `lib.mach` fallback and `mos6502` over `doc/`, `src/cli`, `test/` finds only removal statements and these tests |
| - | legacy result/option/Void APIs | C5a (`33d60001e`) and mach-std#617 | out of C5b's scope | see "C5a: the compiler-side unshim" |

Not in this inventory and left as F3 landed it: the dependency-form leniency
on `link`, `need`, the link filter axes and `export` (owner ruling 2026-09-11
ties the strict root rules for dependencies to the coordinated std 2.0.0
landing).

### C5b verification

All through A, the from-source compiler the v5 stage
(`.wt/v5stage/out/v5/mach`) builds from this branch, against std `e204cb81f`.

- unit suite: 2985 passed, 0 failed (2997 at `33d60001e`); by name 36 removed
  and 24 added. Removed: the 18 mos6502 unit tests that lived in the deleted
  sources (`mach.lang.target.isa.mos6502.{encode,reloc,rules}.*`,
  `mach.lang.target.abi.mos6502.register:*`), the 3 driver tests exercising
  the mos6502 tuple, and 15 tests asserting a 4.30 fallback, warning or note
  (`colon_gt_lowers_identically_to_colon_caret`,
  `deprecated_caret_forms_still_accepted`,
  `the_five_unread_keys_are_accepted_recorded_and_never_read_in_both_forms`,
  `mach_key_is_deprecated_not_unknown`,
  `early_parse_rejection_releases_recorded_keys`,
  `deprecated_manifest_keys_warn_for_the_root_and_each_dependency_naming_the_refusal`,
  `several_profiles_without_a_default_take_the_first_declared_and_say_so`,
  `a_test_goal_without_a_default_takes_the_first_declared_artifact_and_says_so_in_the_request`,
  `a_test_build_that_picks_the_first_declared_artifact_records_the_deprecation_warning`,
  `the_first_declared_fallback_records_the_deprecation_warning`,
  `embed_outside_the_project_root_warns_until_5_0_0`,
  `a_stale_lock_file_is_noted_not_read`, `alias_key_realizes_at_the_key`,
  `dep_closure_conflict_names_chains`, `dep_content_conflicts_unify_mirrors`).
  Added: the 23 tests in the table plus
  `float_gated_out_of_a_freestanding_tuple` (the mos6502 fixture retargeted
  to `rv32imc`/`freestanding`/`ilp32`).
- `sh test/census.sh` all ok; `b-be-7` no longer lists `ARCH_MOS6502`.
- `python3 test/test-corpus.py` ok (the column list no longer carries
  mos6502).
- corpus layers A and B, every remaining column, no golden moves:
  x86_64-linux 408 pass / 0 fail / 0 skip; aarch64-linux 408/0/0;
  riscv64-linux 408/0/0; spirv 281/0/111; riscv32 0/0/204 (the declared
  `* ab` skip, every case still refused by the width legalization pass);
  x86_64-windows 306/0/102; x86_64-darwin 408/0/0; aarch64-darwin 408/0/0.
- link leg x86_64-linux: 140 pass, 0 fail, 0 skip (the embed cases were the
  false refusal `b1cca76f1` fixes).
- vecrows x86_64-linux: 184 probe cells ok, 0 declared exceptions.
- three-generation fixpoint from the v5 stage: A = B = C `e0983e34`.
- cross-builds of the compiler with A for darwin-aarch64 (Mach-O arm64),
  darwin-x86_64 (Mach-O x86_64) and windows-x86_64 (PE32+).
- `mach init` of a scratch bin project under `out/` builds and runs, and a
  `--lib` scaffold builds with its `default = true` artifact. The scaffold
  pins `[dep.std] ref = "branch/main"`, which is std 1.x and is refused by
  this compiler at its first `:^` (the item 1 diagnostic, located in
  `dep/std/src/system/os/secret.mach`); the scratch projects were verified
  with `dep/std` at `e204cb81f`, and the scaffold pin moves with the std
  2.0.0 landing.
