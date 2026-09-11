# L9 editor, deprecation and query acceptance (#3218, #3129)

Acceptance pass for roadmap item L9 on feat/3218, recorded 2026-09-11. L3 to
L8 landed the language; this pass inventories the editor products, the
malformed-buffer recovery, the deprecation surface and the revision contract
for every semantic product the language work added, fills each gap, and
records the test that establishes each cell.

Contracts: `doc/design/tagged-values.md` (language), `doc/design/query-semantic-results.md`
(query products), issue #3129 (deprecation). Legend: `before` names the test
that covered the cell before this pass or the defect found; `after` names the
test added; verdicts are `accepted`, `gap` or `n/a`.

## 1. Editor products

The editor API (`src/lang/editor.mach`) exposes raw products, not rendered
hover text: `expr_type_of`, `decl_type_of`, `resolved_type_of` over the
session type interner, `resolve_of` (symbol per expression and type reference)
and `ast_of`. Every cell below is what a hover, completion or go-to-definition
implementation reads through those products; the cell is accepted when the
data an editor needs is reachable and correct after `analyze` at `PHASE_SEMA`.
No editor test named a tag before this pass.

| Cell | Data path | Before | Verdict | After |
| --- | --- | --- | --- | --- |
| type of a tag binding | `expr_type_of(ident)` is the nominal; `type.is_tag` | untested (rec analogue `analyze:expr_type_bridges_to_field_table`) | gap (test) | `editor.tag:hover_data_covers_the_tag_its_cases_sel_and_the_discriminator` |
| its cases | `type.field_table_for(tag)`: name and payload type per case in declaration order, `TYPE_NIL` payload for a payloadless case | untested | gap (test) | same |
| a `sel` expression | `expr_type_of(sel)` is `bool` | untested | gap (test) | same |
| a payload place `r.value` | `expr_type_of` is the payload type under its guard | L7 sema tests, not through the editor | gap (test) | same |
| case completion after `Type.` | the head resolves (`resolve_of`) to a `SYM_TAG` symbol; `decl_type_of(symbol.decl)` is the nominal; its field table lists the cases | untested; a buffer cut at `Reply.` is malformed (section 2) | gap (test) | `editor.tag:case_completion_after_a_dangling_type_dot_still_lists_the_cases` |
| go-to-definition on a case | `Reply.value{..}`: `expr_type_of(head)` is a case selector, `type.case_selector_case_index` indexes `decl.data.tag_.cases`; `sel r.value`: `ExprSel.case_name` matched against the tag's case table; cross-module the nominal's `TypeOwner.file` names the defining file | untested | gap (test) | `editor.tag:go_to_definition_maps_a_case_use_to_its_declaring_case_span` |
| the discriminator width | `type.tag_discriminator_bytes(tag)` after sema | untested; revision defect (section 4) | gap | same hover test, plus the section 4 revision test |

## 2. Malformed-buffer recovery

Each row is one buffer opened in the editor and analyzed at `PHASE_SEMA`.
Acceptance: the analysis reports a located diagnostic (a span inside the
broken construct), the status is `REJECTED` and never `INTERNAL`, and a
well-formed declaration later in the same buffer still yields editor data
(its expression types resolve).

| Buffer | Before | Verdict | After |
| --- | --- | --- | --- |
| unterminated `tag Reply: u8 {` at end of buffer, then nothing | parser test `parse_tag_decl:recovery_retains_the_following_tag` covers a broken case, not an unterminated body | gap (test) | `editor.recovery:unterminated_tag_body_reports_and_keeps_the_rest_of_the_buffer` |
| unterminated `tag Reply: u8 { empty;` followed by a function | same | gap (test) | same |
| `Reply.value{42` with the closing brace missing | untested | gap (test) | `editor.recovery:case_literal_missing_brace_reports_and_keeps_the_rest_of_the_buffer` |
| `sel` with a missing place: `if (sel) {}` and `if (sel r.) {}` | `parse_sel` reports "`sel` tests one case of a place" on a non-member operand | gap (test) | `editor.recovery:sel_without_a_place_reports_and_keeps_the_rest_of_the_buffer` |
| guard with an unbalanced block: `if (sel r.value) { ret r.value;` then a function | untested | gap (test) | `editor.recovery:unbalanced_guard_block_reports_and_keeps_the_rest_of_the_buffer` |

## 3. Deprecation (#3129)

Nothing on feat/3218 recognizes `#[deprecated]`: the decorator is reported as
unknown. The candidate `3f3427c7` on `archive/3218-candidate-era` implements the
mechanism for `fun`, `rec`, `uni`, `def`, `val`, `var`, `use` and `fwd`, and is
extracted here against the current tree. Two hunks of the candidate are not
taken: the `embed.mach` rename of `escapes_root` to `t_escapes_root` with its
`test/census/real-bools.txt` entry (unrelated to deprecation) and the
`CHANGELOG.md` line (the changelog is edited at release by the coordinator).
The `.github/skills/mach/SKILL.md` table row is taken with the new kinds.

Model, unchanged from the candidate: a notice is `{present, owner: FileId,
message: StrId}` recorded by name resolution on the declaring symbol, carried
on `PublicSymbol` into every importer and into the `Q_EXPORTS` fingerprint,
and reported once per use site by type checking after every body and
instantiation has been inferred. The declaring file never warns on itself. A
`fwd` or `use` annotated with its own notice owns that notice for the exported
name; a clean alias of the same canonical definition keeps no notice, which is
why the imported-symbol cache is keyed by notice as well as origin and name.

Extension for the language work: `tag` declarations take the decorator like
`rec`, and a tag case takes it inside the body (`#[deprecated("msg")] value: i64;`).
A case notice lives on the case's `FieldEntry`, so it travels with the case
table through the typed surface (`Q_TYPED_EXPORTS`) exactly as the case's
payload type does, and is reported at the three case use sites: construction
`Type.case{}`, the test `sel place.case` and the payload place `place.case`.
Descriptor forms (`T.[c]{}`, `sel v.[c]`, `v.[c]`) name no case in source and
warn nowhere.

| Declaration kind | Before | Verdict | After |
| --- | --- | --- | --- |
| `fun`, generic `fun` (each instantiation site once) | candidate test `deprecated_external_sites_decode_messages_and_count_generics_once` | gap (not on branch) | extracted test |
| `ext fun` | candidate covers `fun`; an `ext` function is `DECL_KIND_FUN` | gap | `deprecated_ext_fun_and_tag_declarations_warn_once_per_external_site` |
| `rec`, `uni`, `def`, `val`, `var` | candidate test | gap | extracted test |
| `use` alias, `fwd` re-export, notice ownership, clean aliases | candidate test `deprecated_reexports_keep_annotation_owner_and_do_not_taint_clean_aliases` | gap | extracted test |
| `tag` declaration (value type, generic instantiation, literal head) | none | gap | `deprecated_ext_fun_and_tag_declarations_warn_once_per_external_site` |
| tag case: construction, `sel`, payload place; declaring file silent | none (no decorator on a case in the grammar) | gap | `deprecated_tag_cases_warn_at_construction_sel_and_payload_places_from_other_files` |
| refusals: non-literal, extra, duplicate; `test`, comptime blocks | candidate test `deprecated_attribute_refuses_nonliteral_extra_and_duplicate_arguments` | gap | extracted test, extended with a non-`deprecated` decorator on a case |
| editor: the warning appears in the analysis snapshot with its message | none | gap | `editor.deprecation:external_use_warns_in_the_analysis_snapshot_and_the_declaring_buffer_is_silent` |

## 4. Revision-safe ownership and replay

The language work registered no new query kind. Every semantic product it
added lives inside a product that already exists, so the contract of
`query-semantic-results.md` applies through that owner:

| Product | Where it lives | Owner and finalizer | Revision carrier | Before | Verdict |
| --- | --- | --- | --- | --- | --- |
| case tables | field entries of the tag nominal, captured into `fields.Graph` | `Q_SEMA` (`register_derived_owned`, `q_sema_finalize` runs `sema.dnit_result`, which frees the graph); `Q_TYPED_EXPORTS` bytes | typed surface encodes every node's entries, so a case edit changes the bytes and advances the revision | rec analogue in the driver tests; no tag test | gap (test) |
| tag layouts | computed on demand by `layout.mach` from the installed projection; never stored | n/a (no bytes to own) | the projection inputs (case table, align, packed, discriminator) | | n/a |
| guard results | frames on the checker's stack, dropped when the region closes; the products are the `bool` expression types and the diagnostics of `Q_SEMA` | `Q_SEMA` | `Q_SEMA` | L7/L8 sema tests | accepted |
| canonical tag tables | `TypeInterner.canon_tags`, re-established by `ensure_all_canonical_tags` on every projection reset | session-level, like primitives; retired with the session | not a per-module product | `type.mach` canonical tag tests | n/a |
| the discriminator | `TypeNominal.disc`, written by `set_tag_discriminator` during the declaring module's sema | **not on the projection**: never captured into `fields.Node`, never encoded in the typed surface, never installed, never compared by `same_fields`, never cleared by `field_projection_reset` | **none**: `tag T: u8` to `tag T: u16` leaves `Q_TYPED_EXPORTS` byte-equal, so every dependent keeps its cached `Q_SEMA`, `Q_LOWER` and `Q_CODEGEN` with the old width baked in | defect | gap |
| deprecation notices | on `resolve.Symbol` (declarations) and `type.FieldEntry` (cases); reported as `Q_SEMA` diagnostics of the using module | `Q_RESOLVE` owned; `Q_EXPORTS` and `Q_TYPED_EXPORTS` bytes | notice present, owner and message are fingerprinted, so a message edit advances the dependents | new | gap |

Fix for the discriminator: it becomes projection state like alignment and
packing. `TypeNominal.disc` is removed; the interner keeps a `disc_index`
projection map that `field_projection_reset` clears, `fields.Node` carries
`disc`, `capture_node` reads it, the typed surface encodes it, `install`
applies it through `set_tag_discriminator` and `same_fields` compares it.
Canonical tags are re-established after every reset already.

| Replay cell | After |
| --- | --- |
| A to B to C: edit a tag's case list in the declaring module, revert it; the typed surface revision advances on B and again on C, the dependent recomputes each time, and the case table read after C equals the one read after A | `driver:tag_case_list_edit_and_revert_replay_the_typed_surface_and_its_dependents` |
| discriminator edit `u8` to `u16` advances the declaring module's typed surface and the dependent's `Q_LOWER`; revert advances again and the dependent's layout answers agree with a cold build | `driver:tag_discriminator_edit_advances_the_typed_surface_and_replays_dependents` |
| equal bytes keep the revision: a body-only edit in the declaring module leaves `Q_TYPED_EXPORTS` unchanged | same test |
| deprecation message edit advances `Q_EXPORTS` and re-reports in the dependent; revert restores the first message | `driver:deprecation_message_edit_and_revert_replay_the_public_surface` |
| editor: open A and B, edit A's case list, re-analyze B, revert, re-analyze; no stale case in B's data | `editor.tag:dependency_case_list_edit_and_revert_replay_without_stale_cases` |

Mutation controls, one per invalidation rule added, are recorded in section 6.

## 5. Documentation

`doc/language/tag.md`: `sel` through a pointer place (auto-deref, one level)
and comptime `sel` on constant tags, the two rows L8 left; the layout section
is corrected to the declared discriminator (the page still described an
inferred width). `doc/language/decorators.md`: `#[deprecated]` with the tag
and case forms and the applicability table.

## 6. Gap count and mutation results

Verdict: every cell in sections 1-4 was a gap before this pass (no editor test
named a tag, no deprecation on the branch, the discriminator carried no
revision) and is accepted after it. Sections 1 and 2 are established by the
`editor.tag` and `editor.recovery` tests; section 3 by the `driver:deprecated_*`
and `editor.deprecation` tests; section 4 by the `driver:tag_*_edit*` and
`editor.tag:dependency_case_list_edit*` replay tests and the two query codec
tests (`query.typed_surface`, `query.public_surface`).

Two defects were found and fixed, each with the mutation control that shows its
test failing without the fix:

- **The tag discriminator carried no revision** (this pass). It lived on the
  interned nominal, outside the projection, so a width edit left every
  dependent's cached products unchanged. Fixed by moving it onto the projection
  (`disc_index`, captured into `fields.Node`, encoded in the typed surface,
  compared by `same_fields`). Mutation: dropping the `node.disc` term from
  `same_fields` makes `tag_discriminator_edit_advances_the_typed_surface_and_replays_dependents`
  accept the reverted `u8` build without recomputing (fails), and recording it
  off the case-table path reintroduces the layout-query failures the two seed
  tests `tag_discriminator_is_declared_and_checked` and
  `offset_of_answers_from_the_checked_layout` exhibited.
- **No `#[deprecated]` recognition on feat/3218** (this pass). Mutation:
  removing the tag-case warn arm from `warn_deprecated_uses` makes
  `deprecated_tag_cases_warn_at_construction_sel_and_payload_places_from_other_files`
  see zero notices; keying the imported-symbol cache without the notice makes
  `deprecated_reexports_keep_annotation_owner_and_do_not_taint_clean_aliases`
  taint the clean alias.
