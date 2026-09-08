# Mach v5 execution roadmap

Updated 2026-09-08. Coordinator: Codex. Implementation workers: user-launched
Gemini agents. This is the operational roadmap for Mach #3112 and its complete
cross-repository release scope. The accepted language design remains in
[doc/design/tagged-values.md](doc/design/tagged-values.md).

The two running agents continue their assigned work. No new prompts or workers
are created by this plan. Start the next eligible batch after its inputs are
reviewed and integrated, rather than opening every branch in advance.

## Read this first

The repository has substantial implemented work that has not yet completed issue
acceptance or landed on `dev`. An open issue does not mean its implementation is
absent. Equally, an agent finishing a subtask does not complete its parent issue.

This inventory covers all 31 currently open Mach issues and all six open std
issues. Twenty-nine Mach issues and five std issues are in the v5 gate. The two
remaining Mach issues and std #390 have their own post-v5 batch below. Do not
silently pull those expansions into the v5 release.

There are 43 remaining v5 work items in B0-B10, plus completed G0 and the three
post-v5 items. A work item is a bounded implementation or acceptance deliverable,
not a promise that one model turn will finish it. Plan for roughly 50 to 75 focused
agent work periods for the v5 items, including the two currently running, plus
coordinator reviews at batch boundaries. This is a planning allowance, not a
measured time estimate. Existing implementations may retire acceptance items
quickly. The largest uncertainties are final machine secrecy, tag ABI coverage,
std ownership migration, and Darwin ancillary-descriptor ownership. Re-estimate
after B2 and B5 using actual reports. Do not invent a release date from the
previous agents' eight-minute documentation or 23-minute scalarization runs.

## Current workspace and ownership

| Checkout | Branch | Role | State |
|---|---|---|---|
| Repository root | `dev` | Landed development source | Keep the existing local std checkout intact |
| `.wt/tagged-values-3218` | `feat/3218` | Reviewed v5 integration candidate | Coordinator owns integration, not a third implementation agent |
| `.wt/tag-proofs-3218` | `feat/3218-proofs` | L1, active-case proofs | Gemini agent running |
| `.wt/tag-ir-3218` | `feat/3218-ir-tag` | L2, explicit IR tag representation | Gemini agent running |
| `.wt/v5-integration` | `feat/3112` | Pinned working bootstrap and pre-language candidate | Build dependency, not an active coding lane |
| `/home/octalide/dev/worktrees/mach-main` | `main` | Release source | Keep |

The worker base is `1da0a5286d9557b1bc803dd874f4ed7bda05d339`.
The pre-language candidate is `c5cdd46e20a86e39ba4c8bcc786b20a033fcf2d3`.
The bootstrap executable currently used by the workers is
`.wt/v5-integration/out/linux-x86_64/debug/bin/mach`. Its std source pin is
`ad7add305f107114807df4ca9f59ada51be7998a`. Keep that executable and its sources
until L10 records the replacement migration compiler.

Round-two communication reports belong in
`/tmp/mach-agent-reports-v5-round2/tag-proofs.md` and
`/tmp/mach-agent-reports-v5-round2/tag-ir.md`. Reports and raw logs are temporary
review inputs. Accepted decisions, source hashes and relevant evidence locations
are carried into the work item and PR before their temporary reports are retired.

### Cleanup completed

The initial inventory contained 140 worktrees and 97 local branches. Cleanup
removed 134 inactive worktrees and 91 local branches, leaving the six roles above.
Unmerged historical commits were archived rather than treated as accepted code.
The verified archive is outside the repository:

`/home/octalide/dev/archive/mach-workspace-2026-09-08/`

It contains `history.bundle`, the complete before-inventory, the retirement plan,
verification output, cleanup counts, and copies of the three retired local
transaction-lock files. The bundle retains named historical branch heads and
otherwise detached worktree heads under `refs/workspace-retirement/20260908/`.
Those temporary archive refs were removed from the live repo after verification.
Remote historical branches were not deleted by this local cleanup.

The active integration and bootstrap std clones now own their Git objects rather
than borrowing from a retired worktree. Their origins point to the canonical std
repository. The running agents' working changes were not touched.

To recover an old checkout, inspect `retirement-plan.json`, find its archive ref,
and fetch that single ref from `history.bundle` into one deliberately named
recovery branch. Do not restore the entire old branch collection. Historical
proofs are evidence to reconcile against current source, not a queue of patches
to merge blindly. The prior audit files remain under
`/home/octalide/.cache/mach-audit/`.

## Status and completion rules

- **Running** means one worker owns a named file set and is producing a report.
- **Candidate** means code exists in the integration history but issue acceptance
  or landing remains. Review the implementation before scheduling more code.
- **Partial** means a listed slice is complete and the remaining slice is named.
- **Queued** means there is a concrete work item, but no branch or agent yet.
- **Closed** requires the issue's acceptance, prerequisites, and landed changes.

Every work item ends with a coherent commit, focused validation, review, and an
integration decision. Every completed issue ends with a PR merged to `dev`, issue
closure, and deletion of its finished task branch and worktree. `main` receives
release integration merges from `dev`. Use merge commits. Keep draft PRs while
work remains, then mark them ready. One verification retry is not another issue,
PR, branch, or permanent worktree.

The coordinator performs review and checks actual CI outcomes. The authenticated
account cannot approve its own PR through GitHub's review API. The existing
organization-admin merge allowance may be used after review, as authorized by
the owner. Do not silently disable rules or mislabel absent required checks as
passing. G1 must reconcile the old required `int ...` context names with the
current CI jobs before routine release gating.

## What already exists

| Area | Current evidence | Interpretation |
|---|---|---|
| Accepted tag and failure design | #3217 closed, PR #3232 merged | Complete design, not runtime implementation |
| Fixture repairs | PR #3233 merged, #3119 already closed | Complete repair |
| Pre-language v5 candidate | `c24b9c93`, 2,725 debug and 2,725 release checks recorded | Reuse this evidence for that exact source, not as proof of the current head |
| Tag frontend | Declarations, constructors, canonical types, checked layout and `$is_tag` | Partial #3218, with L1 and L2 running |
| Most recent frontend integration | `1311df15`, 252 frontend checks passed | Includes cross-module canonical specialization |
| Scalarization ownership | `1311df15`, 18 scalarization checks passed | Completed scalarization slice of #3123 |
| Formatter | Canonical formatter and tag/try syntax support, nine focused checks passed | Candidate #3225, final language-form acceptance remains |
| Documentation | Tag generator/linter and 15 language-reference pages reviewed | Partial #3131, migration and executable examples remain |
| Resolver, numerics, query/editor, manifests, plan/check, cache | Existing commits in the v5 candidate | F1-F4 review and land them, do not reimplement from issue titles |
| Native/backend infrastructure | Existing value ABI, sparse liveness, vector catalog, checked layouts and secrecy provenance | N1-N6 identify and complete the actual remaining acceptance |
| std gzip | Complete-stream lifecycle landed at `9d0a3ef`, with recorded native acceptance | S8 is final API migration, not another gzip decoder rewrite |
| std Darwin | Terminal, socket/vector/message calls, VM, ordinary entropy, active CPU discovery and ordinary completion queues have landed checkpoints | S5-S6 own remaining calls and ownership gaps, not those completed conversions |

The old local validation JSON still describes tag semantics as unfinished at an
earlier layout checkpoint. Its commit-specific results remain valid historical
evidence, but its status prose is superseded by this roadmap. The GitHub epic
also contains stale checklist entries and withdrawn-4.30 publication wording.
G1 reconciles those with the accepted source-bootstrap chain.

## Batch schedule

Batches are integration barriers, not days. The table permits up to four
implementation lanes. With two workers, run the named independent lanes in
successive pairs within a batch. The coordinator reviews and lands accepted
work as it completes. A lane may contain sequential items. Do not move a task
across a prerequisite or file-ownership boundary merely to occupy an idle agent.

| Batch | Work lanes | Gate to advance |
|---|---|---|
| B0, now | L1 proofs and L2 IR are running. Coordinator completes G0 cleanup/roadmap and G1 decisions. S0 API inventory can start independently if another worker is available. | Reviewed L1 and L2 interfaces, explicit decision register |
| B1 | L3 tag lowering. F1 then F2 candidate acceptance. N1 backend census/completion. S0 ownership contract inventory if not finished. | Core storage lowering and shared backend contracts frozen |
| B2 | L4 reflection/comptime. L5 native ABI. N2 then N3 linker/RISC-V/vector acceptance. F3 commands/manifests then F5 resource baseline. | Tag frontend/reflection and native transport contracts stable |
| B3 | L7 try semantics. N4 existing module ABI then L6 module tag transport. N5 final native secrecy. F4 cache acceptance. | Try semantic results and native/module transport interfaces stable |
| B4 | L8 try lowering. O1 middle-end ownership. N6 inlining then O2 backend ownership. S1 core std migration after its representation/compiler inputs are usable. | Usable language implementation and migrated std foundation |
| B5 | L9 editor/deprecation/query integration then L10 language acceptance and migration-compiler pin. S2 value/text/codec migration. S3 I/O/filesystem/process/network migration. S4 crypto/synchronization/runtime migration. | Language gate complete, core std public signatures frozen |
| B6 | S5 remaining Darwin interfaces then S6 ancillary ownership. S7 welded secret I/O. S8 gzip final API. Reconcile std #617/#618 ownership inventory across all lanes. | Final retained std API and operation behavior complete |
| B7 | C1 compiler shared foundations first, then C2 frontend, C3 middle/backend, and C4 CLI/driver consumers in parallel. | Compiler builds against the final std candidate with typed failures |
| B8 | C5 legacy removal and paired bootstrap. D1 docs/formatter/examples. D2 closed-catalog audit after source changes settle. | No unowned migration item or obsolete supported surface remains |
| B9 | R1 exact-source native/target acceptance and R2 memory/cache measurements on separate hosts where available. | All release-gate issues have current evidence and are closed in dependency order |
| B10 | R3 version, publish, integrate release, and retire migration branches. | Mach 5.0.0 and std 2.0.0 identify the validated source |
| P, after v5 | P1 embedded RV32 E machines, P2 ELF32 dynamic output, P3 widening integer matmul | Independent additive releases, never prerequisites for v5 |

S1 may prepare edits during B4 but cannot claim a usable migrated library before
L8 supplies working failure handling and records the exact native compiler pin
used to validate that foundation. L10 then certifies the retained-target language
gate and freezes the compiler pin for the remaining migration lanes. B5 workers
may prepare independent std patches while L10 finishes, but their final
semantic/runtime validation uses that pinned migration compiler. S5 and S7 can overlap only after the shared Darwin import and
operation-record interfaces are assigned to one owner. Otherwise run them
sequentially. C2-C4 start from the same reviewed C1 commit.

## Work items and acceptance

Each row states the concrete remaining deliverable. A candidate-review item must
produce either acceptance evidence and landing, or a precise correction in its
owned subsystem. It must not expand into an unlimited testing campaign.

### Coordination and existing candidate

| ID | Issues | Action and exit evidence |
|---|---|---|
| G0 | Mach #3112 | Completed by this roadmap and local cleanup. Maintain the active-work table at every batch boundary. |
| G1 | Mach #3112 | Resolve dependency compatibility/selection policy without assuming that tested commits authorize future versions. Confirm explicit library entry policy and the target/ABI/environment/artifact/debug matrix, especially SPIR-V. Prepare concrete choices for any owner decision still needed. Reconcile withdrawn 4.30 wording, closed checklist entries and required CI job names. Exit with written decisions and a current issue/dependency list. |
| F1 | Mach #3121, #3122, #3229, #3230, #3231 | Review the existing immutable bracket products, numeric coercion/vector division, trivia-independent paths, frontend-only listing and once-per-phase progress changes. Relevant commits include `5463d473`, `5c24103b`, `95e9143b`, `c2fbd635` and `174471c1`. Reuse their existing controls. Land the pre-language candidate in a reviewable PR to `dev`, preserving its coherent commits. Close only the individual issues whose full acceptance is demonstrated. |
| F2 | Mach #3220, #2999 | Complete acceptance of existing transitive query validation, observable diagnostics, owned products, external revisions, selected targets and editor close/retire. Cover A-to-B-to-C invalidation, equal-value diagnostic changes, failures and outstanding product lifetimes. Record the semantic-result extension contract that L9 must obey. |
| F3 | Mach #3222, #3223, #3224 | Review strict profiles/defaults, suffixes, qualified requirements, generated scaffolds, plan selection and frontend check. Demonstrate the promised absence of generators, fetching, backend work and publication. Complete explicit dependency/public-entry decisions through G1. Land and close in order: manifest, plan, then check after F2. |
| F4 | Mach #3221 | Audit existing object cache keys and cross-process reuse. Complete missing compiler/schema, transitive generic/inline, dependency, target, profile, embed, generator/tool/environment and link-input invalidation. Check corruption, interrupted publication, concurrent writers, unavailable-cache behavior, eviction and a real uncached mode. Functional work follows F2 and F5, final closure follows R2 and #2299. |
| F5 | Mach #2299 | Reconcile archived measurements with current sparse liveness and scratch lifetimes. Establish the current many-module, dense-function, aggregate and self-build baseline with serial/parallel provenance. Fix demonstrated scaling cliffs. Preserve the thresholds and workloads for R2, rather than rerunning old mutation campaigns. |

### Backend, linker and ownership

| ID | Issues | Action and exit evidence |
|---|---|---|
| N1 | Mach #2212 | Census current instruction/form/effect descriptions, remaining AArch64 duplication, VReg/PReg identities and registry borrow lifetime. Finish actual gaps in authoritative descriptions and nominal identity checks. Freeze interfaces used by N3-N6. Already split target families are not a rewrite project. |
| N2 | Mach #3113 | Inventory retained ELF, COFF and Mach-O writer sizing/serialization. Finish shared checked extents and nominal SectionId propagation. Independently inspect valid output and prove refusal before publication on alignment, narrowing, overflow and allocation failures. Preserve RV32 static/relocatable output. |
| N3 | Mach #3120, #3127 | Review the existing positive vector capability catalog and canonical RISC-V selection. Fill undeclared retained operation/shape/ABI rows, validate packed/scalar/refused outcomes independently, and prevent generation beyond selected extensions. Use F1's settled numeric behavior. Do not implement E-base RV32 or new dynamic modes here. |
| N4 | Mach #2963, #2940 | Complete acceptance of the existing value-oriented SPIR-V ABI. Retain logical arguments, composites, references and returns without synthetic register allocation. Verify the historical greater-than-16-argument case and legal pointer-to-record parameters. Limits and debug refusals must follow G1's declared environment policy. Then expose the interface L6 needs. |
| N5 | Mach #3126 | Complete validation of emitted machine effects after legalization, selection, allocation, spills, frames, relaxation and encoding expansion. Preserve physical alias, flag, address, indirect-target and latency provenance. Validate late-introduced leaks with independent mutations. Include mixed public-discriminator/secret-payload carriers from L5 and the corresponding logical guarantee after L6. |
| N6 | Mach #3110 | Verify existing same/cross-module inlining and dependency tracking with actual call elimination and bounded growth. Complete the eight transferred atomic wrappers: load, store, cas, fetch_add, fetch_sub, exchange, fence and spin_hint. Preserve ordering, addresses, secrecy, debug mapping and noinline/unsupported-asm controls. Coordinate final std annotations with S4. |
| O1 | Mach #3123 | Complete the allocating middle-end transform inventory and failures in mem2reg, DCE, CSE, algebraic/constant folding, and inlining after N6. Classify genuinely nonallocating paths instead of inventing probes. Scalarization's completed slice is reused. Each allocating path has an actual input-consumption contract and clean teardown under refusal. |
| O2 | Mach #3123 | Cover remaining backend transform allocation owners: selection/legalization, allocation, spill/frame/relaxation and other allocating owners found by census. Reuse the shared fail-at-N apparatus and distinguish durable IR from scratch and error storage. Finish after N1/N5 changes stabilize, then close the complete inventory. |

### Language implementation

| ID | Issues | Action and exit evidence |
|---|---|---|
| L1 | Mach #3218 | Running. Exact-owner selectors, construction/branch proofs, joins and loops, alias/call invalidation, independent snapshots, payload writes/addresses and secrecy. Publish owned semantic metadata for later lowering. |
| L2 | Mach #3218 | Running. Explicit IR tag kind, ordered payload/absence structure, interner ownership, shared checked layout, printing/equality/verification and a consumer audit. No record/pointer fallback. |
| L3 | Mach #3218 | Connect semantic tags to IR and implement locals, globals, arrays, fields, copies, constructors, selectors and payload places. Capture payloads before replacement. Zero tag-owned gaps, inactive suffixes and tail padding without rewriting raw payload representation. Preserve RHS-first destination evaluation, packed value access and typed-address alignment. |
| L4 | Mach #3218 | Implement `$cases`, `$discriminant_of`, descriptor ownership/projection/reconstruction and checked complete-type offset queries. Make generic specialization and comptime tag values agree with runtime representation. Reject payloadless metadata access, foreign descriptors and outer-secret case enumeration. |
| L5 | Mach #3218 | Implement native argument/return transport across SysV64, Win64, AAPCS64 and retained RV64/RV32 conventions. Cover payloadless/nested/packed/overaligned tags, partial carriers, exact object extents and secret byte ranges. Independent caller/callee controls must distinguish logical size from carrier size. |
| L6 | Mach #3218 | After N4, carry logical tag cases and payloads through SPIR-V storage, composites, calls and returns. Keep the module ABI separate from physical native carriers. Validate modules and genuine environment refusals under the support matrix. |
| L7 | Mach #3219 | Implement canonical operand and exact error-binding rules, direct-statement-only success for `err`, mandatory failure exits, scopes and `fin` restrictions. Integrate with L1 proofs. Ordinary tags get no success/failure convention. |
| L8 | Mach #3219 | Lower `try` to once-only operand capture and explicit success/failure control flow. Implement payload extraction, side-effect order, remaining-operand skipping, outer-loop exits and exactly-once cleanup. No fallback initialization or rollback claim. Complete comptime behavior with the same semantics. Record the exact usable native compiler candidate and old std pin before S1 validation. L10 owns full language acceptance. |
| L9 | Mach #3218, #3129, #3220, #2999 | Complete editor type/case data, malformed-buffer recovery, deprecation uses on new declarations, and revision-safe ownership/replay of new semantic products. Revalidate only the query/editor boundaries changed by the language work. |
| L10 | Mach #3218, #3219, #3112 | Integrate the language slices across retained targets and both optimization modes. Cover defaults, aliases, casts, raw validity, reflection, ABI and secrecy together. Close #3218 then #3219 only after their full acceptance and prerequisites. Record a reproducible usable migration compiler built with the old std pin before either source tree adopts the new APIs. |

### Standard library

S0 is an early contract milestone within #617/#618. It does not close those
issues. Representation-dependent code waits for the language implementation.

| ID | Issues | Action and exit evidence |
|---|---|---|
| S0 | std #617, #618 | Inventory every retained public outcome and allocation/resource owner. Assign result, optional absence, closed alternatives, real predicates and native codes deliberately. Define borrowed/error lifetimes, partial effects, address-bound construction, cleanup failures and close/retry state. Assign every module to S1-S4 or explicitly record it unchanged. |
| S1 | std #617, #618 | Migrate allocator, foundational types and collections to canonical forms with explicit ownership. Address-bound owners initialize caller-owned final storage. Freeze core signatures before dependent std lanes. Retain old modules only in the temporary migration branch until C5, never as final compatibility exports. |
| S2 | std #617, #618 | Migrate text, Unicode, encoding, codecs, compression interfaces, math and existing SIMD. Keep invalid input, allocation failure, absence and actual predicates distinct. Own gzip type/error declarations, coordinating implementation with S8. |
| S3 | std #617, #618 | Migrate I/O, filesystem, process, environment and network producers. Preserve partial progress, EOF, would-block, cancellation, native causes and completion lifetimes. Fix full-width Windows exit status and failed GetExitCodeProcess handling, current-directory OOM/native distinctions, environment absence/error distinctions and Unicode comparison failures. Compiler consumers are C4. |
| S4 | std #617, #618, Mach #3110 | Migrate crypto/random, synchronization, clocks, terminal and runtime interfaces not owned by S3. Preserve secret storage, cleanup and address-bound state. Apply and verify the eight atomic wrapper annotations through N6. Coordinate shared OS-facing signatures with S5-S7. |
| S5 | std #415 | From the current source census, finish remaining Darwin process, directory, runtime startup/exit and secret OS boundaries using supported interfaces. Reuse completed terminal, socket/vector/message, VM, ordinary entropy, CPU and completion-queue work. Remove obsolete traps only after the last caller migrates. Native ARM64/x86-64 and independent Mach-O/import/SDK checks verify each changed domain. |
| S6 | std #415, #618 | Implement resource-safe local-byte reception in the presence of unwanted SCM_RIGHTS. Establish a native bound or qualified platform mechanism that observes and closes all installed rights on applicable paths. Cover truncation, pressure, hostile peers, native failure after externalization and unrelated concurrent descriptors. No arbitrary bigger buffer or production process-wide descriptor scan. Reconcile nil wake/close behavior on Linux and Windows under #618. |
| S7 | std #550, #618, Mach #3126 | Implement welded-buffer read/write borrows through terminal native completion, including cancellation settlement, partial progress and cleanup errors. Confine any necessary native ABI escape to the documented backend boundary. Validate no public caller staging alias and correctly timed secret wiping. Use N5's verified compiler contract. |
| S8 | std #418, #617, #618 | Adopt final result/error/ownership forms in the already implemented complete gzip-stream lifecycle. Preserve concatenated members, explicit EOF, draining, sticky failures, counts, truncation/CRC checks and partial-output cleanup. Reuse the independent fixtures and existing first-member regression control. |

S5-S8 finish operation behavior before std #617 and #618 close. Preserve the
published std 1.0.1 contents. The migration candidate is std 2.0.0, with an exact
compiler pin. No std issue is closed merely because its syntax was converted.

### Compiler migration, documentation and release

| ID | Issues | Action and exit evidence |
|---|---|---|
| C1 | Mach #3226, std #617/#618 | Migrate compiler-wide outcome and shared allocation/ownership adapters against final std signatures. Establish one reviewed set of producer/consumer types for C2-C4. Keep raw unions used for actual overlapping representation. |
| C2 | Mach #3226 | Migrate source/session/query, parser/resolver/sema/comptime and editor consumers. Preserve explicit phase classifications, diagnostic ownership and immutable products. Do not replace typed decisions with rendered-message matching. |
| C3 | Mach #3226 | Migrate IR, optimizer, codegen, target and object/link consumers. Preserve allocation extents, consumed-input contracts and secret transport. Reuse O1/O2 controls at the changed ownership seams. |
| C4 | Mach #3226 | Migrate CLI/build/dependency/init/publication and test/run process consumers. Judge Windows success from the full preserved status. Keep native failure separate from absence/OOM, primary errors separate from cleanup errors, and committed effects explicit. No custom Git rollback framework. |
| C5 | Mach #3226, std #617/#618, #3112 | Remove legacy Result/Option/Void helpers after all consumers migrate. Finish the exact removal inventory: old casts, alias/nested deps and root lock, sysv alias, implicit selection/entry defaults, escaping embeds, removed metadata/manifest keys and MOS remnants. Preserve intentional negative fixtures. Pin both source trees and update the source-bootstrap chain, then demonstrate the required self-host fixpoint. Close migration issues only on the coordinated landed state. |
| D1 | Mach #3131, #3225, #3129 | Reconcile grammar, generated help, command schema, manifest reference, migration instructions, ownership docs and generated projects. Exercise behavioral examples with the final compiler/std pair. Finish formatter checks for the complete retained grammar, including descriptor forms, and remove current-spec status contradictions. |
| D2 | Mach #3124 | Re-run the closed-catalog inventory for new semantic, IR, ABI, instruction, callback and capability members. Complete deliberate malformed/unsupported/internal classifications. No unknown-member successful default, and no conversion of valid unsupported capability into a generic internal error. |
| R1 | Mach #3112 and all retained target gates | On exact candidate source, run required native debug/release suites, self-host checks, target conformance, C/object interoperability, debug additivity, secrecy and command examples. Verify nonzero selection counts and required runners. Explain changed goldens with independent evidence, never bulk-bless them. Reuse earlier narrow controls unless relevant source changed. |
| R2 | Mach #2299, #3221 | Measure final uncached/cached memory and time on F5 workloads with compiler/std/host/profile/job provenance. Check storage eviction and resident ownership bounds. Close #2299 before #3221, then update any final dependent issue closure. |
| R3 | Mach #3112 | Resolve all acceptance rows, choose final versioned pins, run the release-source fixpoint, and merge `dev` to `main`. Publish matching Mach 5.0.0/std 2.0.0 tags and artifacts through the authorized release flow. Verify artifact identity. Close #3112, retire obsolete migration branches/worktrees, and preserve bootstrap sources in durable history. |

### Explicit post-v5 batch

| ID | Issues | Action and exit evidence |
|---|---|---|
| P1 | Mach #2859 | Define and implement retained embedded E-base variants with their real reduced-register ISA/ABI, stack and argument/return rules. Independent ABI/encoding and execution controls are required. This is not another ABI name on the full-register machine. |
| P2 | Mach #2894 | Implement ELF32 PIE/shared/dynamic output for chosen RV32 environments, with declared loader/runtime requirements and class-correct structures and relocations. Verify with an independent consumer and suitable execution. |
| P3 | std #390 | Specify widening integer matmul lane/signedness/accumulator/layout/overflow policy from workloads, then implement packed/scalar outcomes and benchmark them. Existing same-width arithmetic is not a missing prerequisite. |

P1 and P2 share RV32 ABI/ELF interfaces, so freeze those together before parallel
implementation. P3 is independent after #3120/#3122. Their exact environment and
performance choices require their own designs and are not silently decided here.

## Issue coverage and closure map

All issue numbers in this table are Mach unless prefixed `std`.

| Issues | Work items | Earliest closure |
|---|---|---|
| #3121, #3122, #3229, #3230, #3231 | F1 | B1 after evidence and landing |
| #3220, #2999 | F2, later extension checks in L9 | B1 for existing contracts, retain L9 obligations under #3218 |
| #3222, #3223, #3224 | G1, F3 | B2 in prerequisite order |
| #3113 | N2 | B2 |
| #3120, #3127 | F1, N3 | B2 |
| #2963, #2940 | N4 | B3 for existing ABI and pointer support, L6 remains owned by #3218 |
| #2212 | N1, N3, N4 and required closed subissues | B3 after complete census and prerequisite acceptance |
| #3126 | N5, L5, L6, S7 boundary evidence | B6 |
| #3110 | F2, N6, S4 | B5 |
| #3123 | Completed scalarization, O1, O2 | B4, with migration seam rechecks in C3 |
| #3218 | L1-L6, L9, L10 | B5 |
| #3219 | L7, L8, L10 after #3218 | B5 |
| #3129 | Existing deprecation candidate, L9, D1 | B8 |
| std #415 | S5, S6 and final std prerequisites | B8 after #617/#618 close |
| std #550 | S7, N5 and final std prerequisites | B8 |
| std #418 | Existing gzip checkpoint, S8 and final std prerequisites | B8 |
| std #617, std #618 | S0-S8, C1-C5 | B8 on coordinated compiler/std landing |
| #3226 | C1-C5, std #617/#618 | B8 |
| #3225 | Existing formatter, L4/L7 syntax, D1, language prerequisites | B8 |
| #3131 | Existing reference/tooling work, D1, completed migration/tooling prerequisites | B8 |
| #3124 | Existing catalog candidate, D2 | B8 |
| #2299, #3221 | F2, F4, F5, R2 | B9 in prerequisite order |
| #3112 | G1, every v5 row, R1-R3 | B10 |
| #2859, #2894, std #390 | P1-P3 | Post-v5 |

The std issues have mutual implementation dependencies, not a reason to retain
legacy APIs indefinitely. S0 freezes contracts, S1-S8 implement producers, C1-C4
migrate consumers, and C5 lands the coordinated removal. Administrative closure
then follows declared issue prerequisites. Likewise, acceptance evidence for
existing query/module ABI contracts can close their issues while new tag-specific
integration remains explicitly owned by #3218.

## Parallelism rules

- L1 owns frontend proof state. L2 owns IR type representation. Neither edits the
  other's files, and neither enables runtime support by assuming the other exists.
- L3/L5/L8 serialize edits to shared lowering and capture code. L4/L7 serialize
  edits to shared semantic/comptime dispatch. Separate target emitters can overlap
  once their shared ABI interface is frozen.
- N1 owns shared machine/effect interfaces before N5. O2 checks the resulting
  owners after those edits. N6 and O1 do not edit inlining simultaneously.
- S1 owns foundational signatures before S2-S4. S3 owns operation/error contracts
  consumed by S5-S7. One owner edits a shared OS/import file at a time.
- C1 lands before C2-C4. Partition the consumer files explicitly and integrate all
  lanes against one std pin. Do not add compatibility fallbacks to keep unfinished
  lanes artificially green.
- Tests run after a coherent implementation or integration checkpoint. Use the
  smallest meaningful filter or native control. Full native suites belong to R1,
  with earlier targeted ABI/secrecy/completion tests where they establish a needed
  contract. R2 performs the final resource comparison once source is stable.
- A discovered defect gets a named owner and a work-item dependency. Do not hide
  it in a report, silently expand another agent's files, or start a new branch
  without assigning its place in this roadmap.

## Sources and maintenance

Scope and acceptance were read from the current
[Mach coordinator](https://github.com/briar-systems/mach/issues/3112), all open
Mach issues and the six open std issues on 2026-09-08. The issue coverage table is
the completeness check. The accepted
[release shape](doc/design/release-shape.md) and
[tooling plan](doc/design/v5-tooling-plan.md) supply existing policy rather than
new design authority. Source and Git history were inspected on the pinned
integration candidate, not inferred only from issue titles.

At each batch boundary the coordinator updates this file with completed IDs,
active owners, the next common base, acceptance locations, and issue/PR closure.
Do not create another handoff file or a competing roadmap. If the actual remaining
work differs, revise the row and its dependent batches before dispatching it.
