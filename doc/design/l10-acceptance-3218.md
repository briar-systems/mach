# L10 integration acceptance (#3218, #3219)

Plan for roadmap item L10 on feat/3218, the last language item. This document
is committed first as the plan and then filled cell by cell; the coordinator
closes #3218 and then #3219 on it. Contract: `doc/design/tagged-values.md`.
Per-lane inventories: `l3-inventory-3218.md`, `l5-abi-inventory-3218.md`,
`l6-spirv-transport-3218.md`, `l8-lowering-acceptance-3218.md`,
`l9-editor-query-3218.md`. Corpus discipline: `corpus-goldens-3218.md`.

## Shape of the matrix

Two tables. Table 1 holds the acceptance bullets the front end decides
(rejections, guards, comptime, editor products); those verdicts are
target-independent, and their one cross-target obligation is that both
pipelines accept and reject the same programs, which one test establishes for
every fixture. Table 2 holds the bullets that are decided by what a target
stores, moves and computes; each row is crossed with every retained target in
`test/engines.conf` and each cell cites the test or corpus case that shows it.

Target columns and what a cell can claim on this host:

| column | modes | evidence a cell may cite |
| --- | --- | --- |
| x86_64-linux | o0, o2, g | suite fixtures executed natively; corpus layers A, B, C |
| aarch64-linux | o0, o2, g | suite fixtures executed under qemu-aarch64; corpus layers A and B (layer C runs on the arm runner in CI, not here) |
| riscv64-linux | o0, o2, g | suite fixtures and corpus layer C under qemu-riscv64; layers A, B |
| spirv | o0, o2 | corpus layers A (spirv-val) and B (spirv-dis); suite module tests. `g` is refused by policy (no debug model) |
| x86_64-windows | o0, o2 | corpus layers A and B only: build-and-decode, never executed here. `g` is declared unsupported and the refusal is verified |
| x86_64-darwin | o0, o2, g | corpus layers A and B only: build-and-decode, never executed here |
| aarch64-darwin | o0, o2, g | corpus layers A and B only: build-and-decode, never executed here |
| riscv32 | declared skip | `test/golden/riscv32/SKIPS` (`* ab`, no 64-bit legalization) |
| mos6502 | declared skip | `test/golden/mos6502/SKIPS` (`* ab`, no float bank, no wide values) |

Cell verdicts: `test` (a suite test, named), `corpus` (a corpus case, named,
with the layers it carries on that column), `A+B` (build-and-decode only),
`skip` (declared in the column's SKIPS file), `policy` (refused by the target
by contract), `gap` (no evidence before this lane; filled by this lane and the
filler named).

## Table 1: front-end verdicts (plan)

Rows are the #3218 and #3219 bullets that the front end decides. To be filled
with the test that establishes each row and the profile-agreement control.

- precise rejection: inactive or unproven payload reads
- precise rejection: conflicting construction (payload on a payloadless case,
  missing payload, extra payload, named payload, bare selector, record form)
- precise rejection: duplicate case names, empty tags, unusable discriminator
- precise rejection: invalid representation conversions (`::` and `:~` through
  arrays, records, unions), identity casts kept
- lexical guards only: arm, exiting chain (`ret`, `brk`, `cnt`), `&&` right
  operand; `||`, `!` open nothing; a non-exiting arm guards nothing after it
- assignment, escaped mutable aliases and potentially modifying calls cannot
  leave a stale guard (whole-value assignment refused; aliases and calls are the
  debug trap's obligation, not a proof)
- `fin` cannot return through a guard
- aliases: module-scope `def` of a tag type constructs and tests as the tag
- secrecy qualifiers: `sel` on an outer-secret tag refused, public case and
  secret payload independent, `:>` strips only outer secrecy
- comptime `sel` and payload reads, constant/runtime agreement
- reflection: `$is_tag`, `$cases`, `$discriminant_of`, descriptor rules
- source locations, malformed-buffer recovery, editor queries, layout
  inspection (`$size_of`, `$align_of`, `$offset_of`)
- `#[deprecated]` on tags and cases
- both profiles accept and reject the same programs

## Table 2: per-target runtime and representation (plan)

Rows to be crossed with the nine columns above:

- local storage: construction, replacement, payload read and write
- arrays of tags: runtime index, element replacement, whole copy
- calls and returns by value with the specified layout (sizes that cross each
  convention's register/memory boundary, float payloads)
- aggregate value semantics: copies are whole, RHS captured before destination
- defaults: zero initialization selects case 0 with a zero payload
- payloadless cases and payloadless tags
- nested tags and tags in records
- generic tags
- alignment: `#[packed]`, `#[align(N)]`, discriminator widths
- partial ABI carriers: logical extent kept apart from carrier width
- comptime and runtime agreement of constant tags and reflection answers
- secrecy: public discriminator carried public, secret payload carried secret
- guarded access under every guard kind at runtime
- debug-profile trap present in o0, absent in o2

## Corpus plan

`test/cases` carries no tag case on any target (L6). This lane adds a `tag`
group with C references so every native column gets executed layer C
evidence where the host executes it and build-and-decode evidence elsewhere:

| case | question |
| --- | --- |
| `tag/construct` | construction of every case form, defaults, replacement, payloadless tag, layout queries, `def` alias, identity cast |
| `tag/guards` | `sel` and guarded read, write and address under every guard kind |
| `tag/nested` | tags in tags, tags in records, payloadless nested cases |
| `tag/call_ret` | tags of 1, 3, 8, 9, 16 and 24 bytes and a float payload through a call and a return, and through a pointer parameter |
| `tag/array` | arrays of tags, runtime index, element replacement, array copy |
| `tag/generic` | one generic tag at three payload types |
| `tag/secret` | a secret payload under a public discriminator, stripped after the test |
| `tag/reflect` | `$cases` walked with `$each`, `sel v.[c]`, codes and offsets folded against the C layout |

Goldens for the new cases are new on every column; no existing golden may
move. riscv32 and mos6502 are already declared away by the `* ab` entries in
their SKIPS files (the counts in those entries are updated). A spirv skip is
declared only with `spirv-val` evidence.

## Deferred to S1

The compiler at this head still seeds canonical `res`, `opt` and `err`
(`seed_canonical_tags` and the sites listed on #3218); S1 removes the seeding
when std 2.0.0 declares the three tags. `doc/language/try.md` describes the
withdrawn `try` expression and is D1's removal.
