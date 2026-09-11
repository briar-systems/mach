# L10 integration acceptance (#3218, #3219)

Acceptance record for roadmap item L10 on feat/3218, the last language item.
Committed first as a plan (d6a239fd7) and filled cell by cell on this lane
(`feat/3218-l10`); the coordinator closes #3218 and then #3219 on it.
Contract: `doc/design/tagged-values.md`. Per-lane inventories:
`l3-inventory-3218.md`, `l5-abi-inventory-3218.md`, `l6-spirv-transport-3218.md`,
`l8-lowering-acceptance-3218.md`, `l9-editor-query-3218.md`. Corpus discipline:
`corpus-goldens-3218.md`. The pinned migration compiler this run was taken on:
`migration-compiler-3218.md`.

## Shape of the matrix

Two tables. Table 1 holds the acceptance bullets the front end decides
(rejections, guards, comptime, editor products); those verdicts are
target-independent, and their one cross-target obligation is that both
pipelines accept and reject the same programs, which one test establishes for
every fixture the suite carries. Table 2 holds the bullets that are decided by
what a target stores, moves and computes; each row is crossed with every
retained target in `test/engines.conf`, and each cell cites the suite test or
corpus case that shows it.

Target columns and what a cell can claim on this host:

| column | modes | evidence a cell may cite |
| --- | --- | --- |
| x86_64-linux | o0, o2, g | suite fixtures executed natively; corpus layers A, B, C |
| aarch64-linux | o0, o2, g | suite fixtures executed under qemu-aarch64; corpus layers A and B (layer C runs on the arm runner in CI, not here) |
| riscv64-linux | o0, o2, g | suite fixtures and corpus layer C under qemu-riscv64; layers A, B |
| spirv | o0, o2 | corpus layers A (spirv-val) and B (spirv-dis); suite module tests. `g` is refused by policy (no debug model) and the corpus verifies the refusal |
| x86_64-windows | o0, o2 | corpus layers A and B only: build-and-decode, never executed here. `g` is declared unsupported and the corpus verifies the refusal. The L5 C control ran under wine, which is not a Windows kernel |
| x86_64-darwin | o0, o2, g | corpus layers A and B only: build-and-decode, never executed here |
| aarch64-darwin | o0, o2, g | corpus layers A and B only: build-and-decode, never executed here |
| riscv32 | declared skip | `test/golden/riscv32/SKIPS` (`* ab`, no 64-bit legalization); the ilp32 ABI classification is unit-level only |
| mos6502 | declared skip | `test/golden/mos6502/SKIPS` (`* ab`, no float bank, no wide values) |

Cell verdicts: `test` (a suite test, named), `corpus` (a corpus case, named,
with the layers it carries on that column), `A+B` (build-and-decode only),
`skip` (declared in the column's SKIPS file), `policy` (refused by the target
by contract, refusal verified), `gap` (no evidence before this lane; the filler
is named).

Suite tests are abbreviated as follows. Every `native` test builds its fixture
for linux-x86_64, linux-arm64 (qemu-aarch64) and linux-riscv64 (qemu-riscv64)
in both profiles and executes it, so one native test is evidence in all six
ISA/profile cells of a row; the `spirv` tests build a module through the whole
pipeline in both profiles and run the pinned `spirv-val` and `spirv-dis` on it.

| key | test |
| --- | --- |
| L3s | `mach.lang.driver:tag_storage_shapes_round_trip_on_every_native_backend` |
| L3n | `mach.lang.driver:nested_tags_and_arrays_of_tags_round_trip_on_every_native_backend` |
| L3z | `mach.lang.driver:tag_replacement_zeroes_gaps_suffix_and_tail_padding_on_every_native_backend` |
| L3o | `mach.lang.driver:tag_assignment_evaluates_the_rhs_before_the_destination_on_every_native_backend` |
| L3p | `mach.lang.driver:packed_and_aligned_tags_access_storage_legally_on_every_native_backend` |
| L3v | `mach.lang.driver:static_tag_values_match_the_runtime_representation_on_every_native_backend` |
| L4c | `mach.lang.driver:comptime_tag_values_agree_with_runtime_representation_on_every_native_backend` |
| L5t | `mach.lang.driver:tag_transport_preserves_logical_extents_through_wider_carriers_on_every_native_backend` |
| L5c | `mach.lang.driver:tag_transport_agrees_with_a_c_callee_and_a_c_caller_on_every_convention` (linux x86_64, arm64, riscv64, and windows x86_64 under wine) |
| L5a | `mach.lang.be.codegen.mir.abi:tag_transport_classifies_as_the_record_of_its_layout_on_every_convention` (sysv64, win64, aapcs64, lp64d, lp64, ilp32d, ilp32; classification only) |
| L5s | `mach.lang.driver:tag_secret_payload_transport_keeps_content_secrecy_and_a_public_discriminator` |
| L5s2 | `mach.lang.driver:tag_secret_payload_carriers_are_secret_and_the_discriminator_test_is_public_on_x86_64_and_aarch64` |
| L6l | `mach.lang.driver:tag_local_construction_sel_and_guarded_access_carry_through_spirv` |
| L6c | `mach.lang.driver:tag_crosses_a_value_call_and_returns_whole_on_spirv` |
| L6r | `mach.lang.driver:tag_in_a_record_and_an_array_walks_constant_ordinals_on_spirv` |
| L6s | `mach.lang.driver:tag_comptime_sel_folds_and_a_secret_payload_is_carried_on_spirv` |
| L7g | `mach.lang.driver:guarded_payload_reads_return_the_payload_on_every_native_backend` |
| L7t | `mach.lang.driver:debug_profile_traps_a_guarded_read_after_the_case_changes` |
| L8g | `mach.lang.driver:tag_payload_access_under_every_guard_kind_on_every_native_backend` |
| L8d | `mach.lang.driver:tag_sel_and_payload_places_auto_deref_a_pointer_on_every_native_backend` |
| L8t | `mach.lang.driver:tag_debug_trap_fires_for_every_guarded_access_kind_on_every_native_backend` |
| L8i | `mach.lang.driver:tag_guarded_access_traps_in_debug_ir_and_not_in_release_ir` |
| L8c | `mach.lang.driver:tag_comptime_sel_and_payload_reads_agree_with_runtime_on_every_native_backend` |
| L8s | `mach.lang.driver:static_tag_initializers_fold_the_constant_they_name_on_every_native_backend` |
| PA | `mach.lang.driver:tag_fixture_corpus_accepts_and_rejects_identically_across_profiles` (every sema fixture, 187 rows, through sema, lowering and codegen under both pipelines) |

Corpus cases are the `tag` group added by this lane (section 3), cited as
`C:<case>`. On x86_64-linux and riscv64-linux a corpus cell is layers A, B and
C (o0 and o2 executed against the C reference, o0 == o2, g verified by
`llvm-dwarfdump`); on aarch64-linux, x86_64-darwin and aarch64-darwin it is
layers A and B with g; on x86_64-windows layers A and B with the g refusal
verified; on spirv layers A (spirv-val) and B (spirv-dis) at o0 and o2.

## 1. Front-end verdicts

Each row is one cell; the verdict is target-independent. PA is the control
that both pipelines agree on every fixture named here.

| # | bullet | evidence | verdict |
| --- | --- | --- | --- |
| 1.1 | inactive or unproven payload reads rejected precisely (outside a guard, under `\|\|`, before the test, after a non-exiting arm, through a foreign descriptor) | `fe.sema:payload_access_outside_a_guard_is_rejected`, `fe.sema:tag_payload_access_requires_guard`, `fe.sema:descriptor_projection_obeys_the_named_case_guard_rules`; comptime twin `fe.sema:comptime_sel_tests_constant_tags` (a payload read of the case a constant does not hold is a located error) | test |
| 1.2 | conflicting construction rejected (payload on a payloadless case, missing, extra, named payload, bare selector, record form) | `fe.sema:tag_construction_names_the_type_and_the_case`, `fe.sema:tag_record_literal_form_is_withdrawn`, `fe.sema:case_selector_usage_restrictions`, `fe.sema:canonical_tags_payload_validation`, `fe.sema:typed_literals_distinguish_cases_from_vector_elements` | test |
| 1.3 | duplicate or invalid discriminants rejected (duplicate case names, empty tag, a declared width too narrow for the case count, a non-integer or signed width) | `fe.sema:tag_declarations_reject_invalid_case_recipes`, `fe.sema:tag_discriminator_is_declared_and_checked`, `fe.sema:tag_discriminator_declarations_are_rejected_when_unusable`, `layout.tag:discriminator_width_boundaries`, `me.ir.type.tag:discriminator_width_boundaries`, `fe.parser.decl.parse_tag_decl:generic_cases_keep_payload_presence_and_order` | test |
| 1.4 | invalid representation conversions rejected: `::` and `:~` when either side contains a tag, through records, arrays and union alternatives; identity casts kept | `fe.sema:tag_representation_cannot_be_forged_through_casts` (`x::Reply`, `x::Box` with a tag field, `x::Box` with a tag union alternative, `x:~Reply`; `x::Reply` and `x:~Reply` on the same type accepted), `fe.sema:canonical_tags_casts_and_contextual_names`; the identity cast executes in C:construct (`a::Reply`) and L8s (`r::Reply` in a static initializer) | test |
| 1.5 | lexical guards only: a chain arm whose condition is exactly `sel P.c`, the rest of a block after an all-exiting chain (`ret`, `brk`, `cnt`), the right operand of `&&`; `\|\|`, `!` and everything else open nothing; the operand is a place followed by one case | `fe.sema:sel_arms_guard_their_block`, `fe.sema:exiting_chains_guard_the_rest_of_the_block`, `fe.sema:and_guards_only_its_right_operand`, `fe.sema:sel_tests_one_case_of_a_place`, `fe.sema:sel_rejects_non_places_and_unknown_cases`, `fe.sema:sel_auto_derefs_a_pointer_place` | test |
| 1.6 | assignment, escaped mutable aliases and potentially modifying calls cannot leave a stale guard: whole-value assignment to the guarded place is refused inside the guard (also `b.r = ...` under `sel b.r.value` through a pointer receiver); mutation through an alias or a call is the raw-memory obligation and is caught by the debug trap, not by a proof | `fe.sema:guarded_place_rejects_whole_value_assignment`; L7t, L8t (the case switched through a raw pointer between the guard and each access kind traps under debug and reads through under release), L8i (one `unreachable` per guarded access in debug IR, none in release) | test |
| 1.7 | a `ret` inside `fin` cannot cross a guard | `fe.sema:fin_cannot_return_through_a_guard` | test |
| 1.8 | aliases: a module-scope `def` of a tag type constructs, tests and copies as the tag | `fe.sema:tag_construction_through_qualified_and_aliased_heads` (`def A: Reply; A.value{v}`), `fe.sema:tag_representation_cannot_be_forged_through_casts` (`def Alias: Reply; Alias.empty{}`); executed on every column by C:construct (`def R: Reply`) | test |
| 1.9 | secrecy qualifiers with independent controls: `sel` on an outer-secret `^Tag` refused ("outer `^` secrecy protects the selected case"), a public case tested and its payload read beside a secret payload, `:>` strips only outer secrecy | the outer-secret `sel` row of the sema fixture table (`src/lang/driver/tests.mach`, `TagFixture` at the `^Reply` row) through PA; `fe.sema.generics.contains_secret_deep:*`, `type.intern_secret:*`; carriers in table 2 row 12 | test |
| 1.10 | comptime `sel` and payload reads on constant tags agree with runtime; a non-constant place is a located diagnostic | `fe.sema:comptime_sel_tests_constant_tags`, `driver:comptime_sel_on_a_mutable_or_foreign_place_is_reported_at_the_condition`, L8c, L4c, L3v, L8s | test |
| 1.11 | reflection: `$is_tag`, `$cases` with `name`, `has_payload`, `type`, `offset`, `code`, `$discriminant_of`; descriptor construction and tests; foreign descriptors refused; no compiler knowledge of `res`/`opt`/`err` needed | `fe.sema:is_tag_queries_public_nominal_shapes`, `fe.sema:cases_enumerates_owner_qualified_case_descriptors`, `fe.sema:descriptor_projection_obeys_the_named_case_guard_rules`, `me.lower:cases_projection_and_construction_lower_and_codegen`, `driver:tag_reflection_needs_no_compiler_knowledge_of_res_opt_err`; executed on every column by C:reflect | test |
| 1.12 | source locations, malformed-buffer recovery, editor queries and layout inspection reflect the new types | `editor.tag:hover_data_covers_the_tag_its_cases_sel_and_the_discriminator`, `editor.tag:case_completion_after_a_dangling_type_dot_still_lists_the_cases`, `editor.tag:go_to_definition_maps_a_case_use_to_its_declaring_case_span`, `editor.tag:dependency_case_list_edit_and_revert_replay_without_stale_cases`, `editor.recovery:unterminated_tag_body_reports_and_keeps_the_rest_of_the_buffer`, `editor.recovery:case_literal_missing_brace_reports_and_keeps_the_rest_of_the_buffer`, `editor.recovery:sel_without_a_place_reports_and_keeps_the_rest_of_the_buffer`, `editor.recovery:unbalanced_guard_block_reports_and_keeps_the_rest_of_the_buffer`, `fe.parser.decl.parse_tag_decl:recovery_retains_the_following_tag`; layout: `fe.sema:tag_declarations_have_checked_payload_layouts`, `layout.tag:common_payload_offset_packing_and_outer_alignment`, `layout.tag:payloadless_nested_overflow_and_recursive_cases`, `driver:tag_discriminator_edit_advances_the_typed_surface_and_replays_dependents`, `driver:tag_case_list_edit_and_revert_replay_the_typed_surface_and_its_dependents`, `driver.query.typed_surface:discriminator_and_case_notices_round_trip_and_move_the_bytes`; `$size_of`/`$align_of`/`$offset_of`/`$discriminant_of` executed against the C layout on every column by C:layout | test |
| 1.13 | `#[deprecated]` on a tag and on a case, warned once per external site at construction, `sel` and the payload place; declaring file silent | `driver:deprecated_ext_fun_and_tag_declarations_warn_once_per_external_site`, `driver:deprecated_tag_cases_warn_at_construction_sel_and_payload_places_from_other_files`, `driver:deprecated_external_sites_decode_messages_and_count_generics_once`, `driver:deprecated_reexports_keep_annotation_owner_and_do_not_taint_clean_aliases`, `driver:deprecated_attribute_refuses_nonliteral_extra_and_duplicate_arguments`, `editor.deprecation:external_use_warns_in_the_analysis_snapshot_and_the_declaring_buffer_is_silent`, `driver.query.public_surface:deprecation_owner_and_message_are_observable`, `driver:deprecation_message_edit_and_revert_replay_the_public_surface` | test |
| 1.14 | both profiles accept and reject the same programs | PA (187 fixtures; the debug verdict, the release verdict and the expected verdict must agree); every native and spirv test above builds under both pipelines; corpus layer C requires o0 == o2 on every executed case | test |

## 2. Per-target runtime and representation

Rows are the bullets a target decides; columns are the nine retained targets.
`lin3` abbreviates the three linux columns (x86_64-linux, aarch64-linux,
riscv64-linux), each of which a `native` test covers in both profiles; the
corpus cite in a `lin3` cell is layers A, B and C on x86_64-linux and
riscv64-linux and layers A and B on aarch64-linux. `win` is x86_64-windows,
`dar2` the two darwin columns, `rv32`/`mos` the two declared-skip columns.

| # | bullet | lin3 | spirv | win | dar2 | rv32, mos |
| --- | --- | --- | --- | --- | --- | --- |
| 2.1 | local storage: construction of every case form, replacement, payload read and write, `?place.case` | L3s, L3z; C:construct, C:guards | L6l; C:construct, C:guards (both profiles) | **gap** → C:construct, C:guards (A+B) | **gap** → C:construct, C:guards (A+B, g) | skip |
| 2.2 | arrays of tags: zero elements, runtime index, element replacement, whole copy, array in a record through a call | L3n; C:array | L6r; C:array | **gap** → C:array (A+B) | **gap** → C:array (A+B, g) | skip |
| 2.3 | calls and returns by value with the specified layout: 1, 3, 8, 9, 16 and 24 bytes, f64 and f32 payloads, a mixed f64/i64 payload set, a nested tag, three tags in one call, a pointer parameter | L5t, L5c, L5a; C:call_ret | L6c; C:call_ret | L5a (win64 classification), L5c (wine); C:call_ret (A+B) | L5a (the darwin vtables are the linux sysv64 and aapcs64 ones); C:call_ret (A+B, g) | skip (L5a classifies ilp32d/ilp32 at unit level; no execution engine) |
| 2.4 | aggregate value semantics: copies are whole and independent of the source, the RHS is captured before the destination is overwritten | L3o, L3s; C:construct (`r = Reply.value{value_of(r) + 1}`, copy then write to the copy), C:nested | L6l (one `OpLoad`/`OpStore` per copy); C:construct | **gap** → C:construct, C:nested (A+B) | **gap** → C:construct, C:nested (A+B, g) | skip |
| 2.5 | defaults: zero initialization selects case 0 with a zero payload for locals, arrays, omitted record fields and nested tags | L3s, L3n; C:construct, C:array, C:nested | **gap** → C:construct, C:array, C:nested (a `var` with no initializer is a `OpConstantNull` store) | **gap** → C:construct, C:array, C:nested (A+B) | **gap** → same (A+B, g) | skip |
| 2.6 | payloadless cases and payloadless tags | L3s, L5t (`P1`); C:construct (`Flag`), C:call_ret (`P1`) | L6l (the unit composite); C:construct, C:call_ret | **gap** → C:construct, C:call_ret (A+B) | **gap** → same (A+B, g) | skip |
| 2.7 | nested tags, tags in records, records in tags, three levels | L3n; C:nested | L6r; C:nested | **gap** → C:nested (A+B) | **gap** → C:nested (A+B, g) | skip |
| 2.8 | generic tags at scalar, float, record and nested-tag arguments | L4c (`Box[u32]`, `Box[Pair]`); C:generic | **gap** → C:generic | **gap** → C:generic (A+B) | **gap** → C:generic (A+B, g) | skip |
| 2.9 | alignment: `#[packed]`, `#[align(N)]`, `u8`/`u16`/`u32` discriminators, float payloads, sizes and offsets against the C layout | L3p, L3s; C:layout | **gap** → C:layout | **gap** → C:layout (A+B) | **gap** → C:layout (A+B, g) | skip |
| 2.10 | partial ABI carriers: the logical extent kept apart from the carrier width (a 3-byte tag in an 8-byte register, a 9-byte tag in two registers, padding-only eightbytes) | L5t, L5c, L5a; C:call_ret (`T3`, `T9`, `O24`) | policy: a whole-module target has no physical carriers (L6 section 3; L6c shows one `CLASS_VALUE` slot) | L5a (win64 `byref`/`sret` for 3, 9, 16, 24 bytes), L5c (wine); C:call_ret (A+B) | L5a; C:call_ret (A+B, g) | skip |
| 2.11 | comptime and runtime agreement: constant tags, static initializers, reflection answers | L4c, L8c, L3v, L8s; C:reflect | L6s; C:reflect | **gap** → C:reflect (A+B) | **gap** → C:reflect (A+B, g) | skip |
| 2.12 | secrecy: a public discriminator carried public, a secret payload carried secret through a call and a return, an outer-secret tag copied by its fixed extent and stripped | L5s (all three ISAs), L5s2 (x86_64, aarch64); C:secret | L6s; C:secret | **gap** → C:secret (A+B) | **gap** → C:secret (A+B, g) | skip |
| 2.13 | guarded access at runtime under every guard kind: arm, exiting chain by `ret`, `brk`, `cnt`, `&&` right operand, three-case chain, nested, pointer auto-deref and explicit deref; `sel` bound to a value | L7g, L8g, L8d; C:guards | L6l; C:guards | **gap** → C:guards (A+B) | **gap** → C:guards (A+B, g) | skip |
| 2.14 | debug-profile discriminator trap present at opt 0 and absent at opt 2 | L7t, L8t, L8i; every corpus o0 cell executes with the checks and never traps | L6l (`OpUnreachable` blocks in debug, none in release); corpus o0 builds and validates with them | **gap** → corpus o0 A+B builds with the checks (not executed) | **gap** → same (A+B) | skip |
| 2.15 | `-g` debug information over tag-bearing code verifies | **gap** → C:tag/* layer A `g` (`llvm-dwarfdump --verify`) | policy: no debug model; the corpus verifies the refusal on every case | policy: declared unsupported; the corpus verifies the refusal on every case | **gap** → C:tag/* layer A `g` (A+B) | skip |

Every `lin3` cell also gained corpus evidence from this lane; the cells marked
`gap` had no evidence of any kind before it.

### Totals

| | cells | evidenced before this lane | added by this lane | refused by policy or declared skip |
| --- | --- | --- | --- | --- |
| table 1 | 14 | 14 | 0 (five rows gained executed corpus evidence) | 0 |
| table 2 | 135 | 58 | 44 | 33 |
| total | 149 | 72 | 44 | 33 |

Table 2 arithmetic: `lin3` rows 2.1 to 2.14 are 42 cells evidenced before;
spirv rows 2.1, 2.2, 2.3, 2.4, 2.6, 2.7, 2.11, 2.12, 2.13, 2.14 (10) before,
rows 2.5, 2.8, 2.9 (3) added, rows 2.10 and 2.15 (2) policy; windows rows 2.3
and 2.10 (2) before, twelve rows added, row 2.15 policy; each darwin column
rows 2.3 and 2.10 (classification, 2) before and twelve added, plus row 2.15
added; `lin3` row 2.15 (3) added; riscv32 and mos6502 15 rows each (30)
declared skip.

## 3. Corpus: the `tag` group

`test/cases` carried no tag case on any target (L6). This lane adds nine cases
with C references (`test/ref/tag`), each folding every selected case and
payload so a wrong code, a stale payload, a misplaced common offset or a wrong
stride moves the checksum. All nine execute on x86_64-linux (native) and
riscv64-linux (qemu) at o0 and o2 against the host C answer, build and decode
on aarch64-linux, x86_64-windows, x86_64-darwin and aarch64-darwin, and build
and validate on spirv at both profiles. Goldens are new on every column (63
files, seven native columns and spirv times nine cases); no existing golden
moved (`git status` over `test/golden` shows only the new files and the three
SKIPS edits). riscv32 and mos6502 are already declared away by their `* ab`
entries, whose counts now read 101.

| case | question |
| --- | --- |
| `tag/construct` | every construction form, the zero default, a payload write under a guard, replacement by a payloadless case, RHS-first replacement, a `def` alias, an identity cast, whole copies |
| `tag/layout` | sizes, alignments, offsets and `$discriminant_of` widths of natural, `#[packed]`, `#[align(16)]`, `u16`- and `u32`-discriminated and float-payload tags against the C layout, then constructed, read and written |
| `tag/guards` | `sel` and the guarded read, write and address under every guard kind of the contract |
| `tag/nested` | tags in tags, tags in records, records in tags, payloadless cases at every level |
| `tag/call_ret` | the twelve L5 shapes minus the packed and over-aligned ones through a call and a return, three tags in one call with a trailing scalar, a pointer parameter |
| `tag/array` | arrays of tags: runtime index, element replacement, payload write through an index, whole copy, an array in a record by value, a `u16` discriminator's stride |
| `tag/generic` | `Box[T]` at `u8`, `i64`, `f64`, a record and `Box[i64]`, with the instantiation sizes |
| `tag/secret` | `^u64` payload under a public discriminator through a call and a return, stripped after the test; an outer-secret `^SecretReply` copied whole and stripped |
| `tag/reflect` | `$cases` walked with `$each`, `sel v.[c]`, `v.[c]`, `T.[c]{}`, `code` and `offset` against `offsetof`, `$is_tag` and `$discriminant_of` folded |

One more case, `cmp/ret_chain_inline`, is not a tag case: it records the
spirv defect found while reducing the four tag cases that were refused on
spirv at o2 (section 5).

## 4. Documentation

`doc/language/tag.md` describes the shipped contract: the declared
discriminator, `Type.case{}`, `sel`, lexical guards, the debug trap, layout,
casts, reflection, `#[deprecated]` on tags and cases, and the std failure tags
`res`, `opt` and `err` as std's declarations. The `try` section and the
try.md links are gone from it. `doc/language/decorators.md` carries the
`#[deprecated]` forms and the applicability table (L9). `doc/manifest.md`
carries nothing tag-specific and nothing that contradicts the contract. The
CHANGELOG's Unreleased section now carries the tagged-value entry and the
`#[deprecated]` entry.

Two things the documentation states as transitional, on purpose:

- The compiler at this head still seeds canonical `res`, `opt` and `err`
  (`seed_canonical_tags`, `reject_builtin_named_type`, `scope_lookup_chain`,
  `type_spelling`, `CanonTagFamily`/`ensure_canon_tag`/`canon_tags`,
  `ensure_all_canonical_tags`, the generics and infer canonical instantiation
  paths, and 21 driver tests naming them bare, as listed on #3218), and
  refuses a module that declares one of the three names
  (`fe.sema:canonical_tags_casts_and_contextual_names`). S1 (mach-std#617)
  removes the seeding when std 2.0.0 declares them; reflection is already
  name-blind (`driver:tag_reflection_needs_no_compiler_knowledge_of_res_opt_err`).
- `doc/language/try.md` describes the withdrawn `try` expression and is D1's
  removal, together with the `try` mentions in `doc/language/README.md`,
  `expressions.md`, `grammar.md`, `operators.md` and `statements.md`. tag.md
  no longer links to it.

## 5. Found by this lane

- **spirv o2 structurizer refuses an inlined three-way early-return chain
  (#3275).** Four of the nine tag cases were refused on spirv at o2 with "a
  branch that leaves a control-flow construct other than through its merge,
  break, or continue edge is not expressible in the SPIR-V target", in helpers
  of the form `if (sel a) { ret x; } or (sel b) { ret y; } ret z;`. Reduced
  with no tag at all: `fun pick(v: u32) u32 { if (v == 1) { ret 7; } or (v ==
  2) { ret 9; } ret 65535; }` inlined twice into one function, or once into a
  loop, reproduces it; a two-way chain and the single-return form both build.
  The tag helpers were spelled with one return so the spirv column carries all
  nine cases, and the shape itself is `cmp/ret_chain_inline`, executed on every
  native column and declared a MACH DEFECT skip on spirv (`ab:o2`, #3275). It
  is not fixed here: the fix is a structurizer change under `cfg.mach:verify`,
  which can move the existing spirv goldens and is outside an acceptance lane.
- **Assignment of a public expression to a secret place types the expression
  against the destination.** `r.value = (r.value:>u64) * 3` on a `^u64` payload
  is refused as constant-time arithmetic outside `#[oblivious]`, while
  `val open: u64 = (r.value:>u64) * 3; r.value = open;` and the construction
  form `SecretReply.value{(r.value:>u64) + 1}` are accepted. The same refusal
  applies to `var x: ^u64; x = a + 3;` with a public `a`, so it is the existing
  secrecy rule for assignment and not a tag defect; `tag/secret` is spelled
  through the public local. Recorded here so the S1/std authors do not read it
  as a payload-place rule.
- `bool` is a std `def`, so a corpus case (which reaches no std) binds a `sel`
  result to `u8`; the contract's "ordinary `bool`" is that alias.

## 6. Verification

Taken in one process on the final measured tree (see
`migration-compiler-3218.md` for the commit, the seed hash and the exact
commands), with the from-source compiler built by the 4.30.0 seed at
`out/audit/mach` (debug profile) and `out/audit/mach-release`.

| bar | result |
| --- | --- |
| full suite, debug from-source compiler | 2911 passed, 0 failed, 2911 total (unchanged from the feat/3218 base: this lane adds no suite test) |
| full suite, release from-source compiler | 2911 passed, 0 failed, 2911 total (agrees with the debug build) |
| `sh test/census.sh` | every census ok (b-fs-1, b-fs-1-publication, b-diag-1, p-4-lower-scope, b-fe-4-scan, b-fe-4-walk, q-kinds, real-bools 111) |
| corpus layers A and B, all nine columns | 2625 pass, 0 fail, 623 skip over the nine columns (per-column counts in `migration-compiler-3218.md`); every skip is a declared SKIPS entry or a verified `g` refusal |
| corpus layer C, x86_64-linux and riscv64-linux | 408 pass, 0 fail, 0 skip (102 cases, o0 and o2 on each of the two executing columns, every checksum equal to the C reference) |
| link leg x86_64-linux | 140 pass / 0 fail / 0 skip over 140 cells (debug and release), re-run after merging origin/feat/3218 at c14511dcd |
| link leg riscv64-linux | not run: the host lacks riscv64 libc headers |
| vecrows x86_64-linux | ok, 184 probe cells, 0 declared exceptions |
| fixpoint | A `70dd51d4…`, B `d70175ef…`, C `d70175ef…`; B == C byte-identical (`cmp`) |

No new rejection or guard rule was added by this lane, so no mutation control
is owed beyond the ones the per-lane documents record; the corpus's own
control is the C reference and the o0/o2 agreement.
