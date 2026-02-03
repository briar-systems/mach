# Description

Restore full parity between bootstrap and self-hosted Mach compilers to achieve a complete `cmach → imach → mach → mach` bootstrap chain. Fix root causes only; remove all workarounds and align behavior, feature inclusion, and codegen between the two implementations.

# Context

This task advances the `GAMEPLAN.md` milestone "mach can compile itself (full bootstrap)" and enforces strict parity. Any discontinuities (e.g., `x86asm.mach` in self-hosted but not in bootstrap) must be resolved by aligning both implementations.

# Outline

- [ ] **Phase 1: Parity audit and cleanup**
  - [ ] Inventory all self-hosted changes marked as workarounds (aggregate copy, Result-by-value, by-value small structs, uninitialized field guards).
  - [ ] Inventory feature discontinuities between bootstrap and self-hosted (e.g., `x86asm.mach` inclusion, codegen paths).
  - [ ] Define the exact parity target for each discontinuity (add to bootstrap or remove from self-hosted).
  - [ ] Remove all workaround code paths after root-cause fixes are in place.

- [ ] **Phase 2: Root-cause fixes in bootstrap codegen**
  - [ ] Confirm bootstrap audit/fixes are complete before mirroring to self-hosted.
  - [ ] **lower.c audit and fixes**
    - [ ] Return lowering: verify sret detection, small-aggregate register returns, and call-return forwarding.
    - [ ] Call lowering: confirm sret buffer allocation, hidden arg0 placement, and small-aggregate call result handling.
    - [ ] Param handling: verify two-pass save (scalar/fp then aggregate) preserves argument registers and stack alignment.
    - [ ] Assignments: ensure all aggregate stores (ident/index/field/deref/global) use `emit_aggregate_copy` with call-result exceptions.
    - [ ] Symbol/expr lowering: confirm aggregate globals and linkage fallbacks are treated as addresses consistently.
  - [ ] **isel.c audit and fixes (x86_64)**
    - [ ] Call argument classification: ensure stack vs register placement matches ABI and sret arg0 handling.
    - [ ] Return handling: verify RAX/RDX/XMM0 moves for small aggregates and vreg slot stores.
    - [x] Odd-size small aggregates: implement chunked by-value load/store in isel to preserve C ABI.
    - [x] Document odd-size small aggregate by-value moves in `doc/masm/abi.md`.
    - [ ] Vreg stack layout: confirm `get_vreg_offset` and stack frame sizes are consistent with lowered vreg usage.
      - [x] Review `scan_function` max vreg index calculation for multi-slot vregs.
      - [x] Verify `get_vreg_offset` assigns offsets consistent with vreg slot sizing and alignment.
      - [x] Validate total stack size computation vs. allocated vreg region and 16-byte alignment.
      - [x] Check for any code paths that assume single-slot vregs when size > 8.
    - [ ] Memory operands: validate base vreg handling and scratch usage in `load_operand`/`store_vreg`.
      - [x] Audit base vreg resolution paths and ensure scratch regs are not clobbered mid-address calc.
      - [x] Verify index vreg handling (when present) mirrors base vreg resolution and avoids conflicts.
      - [x] Review chunked move helpers for mem operands with base/index and confirm no overlap with dest.
      - [x] Confirm mem operand sizes match vreg slot sizes to avoid over-read/over-write.
    - [ ] Odd-size aggregate tests (bootstrap):
      - [ ] Add return-by-value coverage for 3/5/6/7 byte aggregates.
      - [ ] Add by-value argument passing coverage for 3/5/6/7 byte aggregates.
      - [ ] Add aggregate init/store coverage for fields, array elems, and globals from call results.
      - [ ] Add mem operand copy coverage for base/index addressing with odd sizes.
  - [ ] **encode.c audit**
    - [ ] Verify instruction encodings/relocations preserve lowered semantics for call/ret/mem ops.
  - [ ] Skip bootstrap unit tests (bootstrap doesn't have unit tests; use self-hosted test framework).
  - [ ] Verify bootstrap output matches self-hosted behavior for affected patterns.

- [ ] **Phase 3: Root-cause fixes in self-hosted codegen**
  - [ ] Mirror bootstrap vreg slot reservation changes in lowering (multi-slot vregs).
  - [ ] Mirror x86_64 isel fixes: odd-size chunked loads/stores and mem base/index vreg resolution.
  - [ ] Align self-hosted lowering and isel behavior with bootstrap semantics.
  - [ ] Ensure `.text` emission and relocation handling matches bootstrap expectations.
  - [ ] Validate calling convention implementation (args, returns, stack alignment, caller/callee save).

- [ ] **Phase 4: Remove parity discontinuities**
  - [ ] Align inline asm and x86asm handling across bootstrap and self-hosted.
  - [ ] Remove or add modules to ensure identical feature surface and behavior.
  - [ ] Confirm both compilers emit equivalent code for representative inputs.

- [ ] **Phase 5: End-to-end bootstrap validation**
  - [ ] `cmach → imach` builds successfully.
  - [ ] `imach → mach` builds successfully.
  - [ ] `mach → mach` builds successfully and outputs a working binary.
  - [ ] Run representative builds to confirm parity and stability.

# Log

## 2026-02-02 09:38
- fixed array type length resolution to require a constant integer literal (matches bootstrap behavior).
- removed pointer-to-array assignment workaround now that array lengths are parsed correctly.

## 2026-02-02 10:54
- continued session after chat interruption.
- expanded Phase 2 outline with detailed lower.c, isel.c, and encode.c audit steps.

## 2026-02-02 10:58
- fixed call argument lowering for small aggregates: avoid dereferencing call results when passing by value.

## 2026-02-02 11:00
- updated x86_64 isel scan to account for multi-slot vregs when sizing the stack.

## 2026-02-02 11:09
- fixed x86_64 isel load_operand sizing to avoid over-reading vreg stack slots.

## 2026-02-02 11:24
- completed initial encode.c audit pass; no edits required.

## 2026-02-02 11:30
- confirmed with user: finish bootstrap compiler audit/fixes first, then mirror to self-hosted.

## 2026-02-02 10:02
- confirmed with user: proceed with bootstrap aggregate/argument/return audit and fixes, skip bootstrap unit tests.

## 2026-02-02 09:26
- completed initial parity audit scan (no fixes applied yet).
- found explicit workarounds in `src/compiler/masm/lower.mach` (Result return temp, direct struct field assignment to avoid copy, masm context manual init).
- found sema debug prints and helper functions in `src/compiler/sema.mach` that should be gated or removed for parity.
- found a type-system workaround in `src/compiler/type.mach` allowing pointer-to-array assignment without length checks.
- identified discontinuity: self-hosted `src/compiler/masm/isa/x86asm.mach` exists while bootstrap uses `boot/src/compiler/masm/isa/x86_64/asm.c` (needs strict parity alignment).

## 2025-02-02 00:00
- Reset task scope to strict parity and root-cause fixes only.
- Explicitly removed allowance for workarounds and temporary solutions.
- Aligned task objective with `GAMEPLAN.md` full bootstrap milestone.

## 2026-02-02 17:39
- updated `boot/src/compiler/masm/lower.c` var/val aggregate init handling to copy non-call aggregates by value while preserving the small-aggregate-from-call exception.

## 2026-02-02 18:06
- updated `boot/src/compiler/masm/lower.c` aggregate literal init handling to store small aggregates returned by calls directly (array elems, struct fields), avoiding address-based copies.

## 2026-02-02 18:08
- updated `boot/src/compiler/masm/lower.c` call/return lowering to treat small arrays as by-value aggregates (include TYPE_ARRAY in small/large aggregate checks).

## 2026-02-02 18:13
- user requested explanation and recommendation for handling odd-sized small aggregates (by-value chunked handling vs by-reference).

## 2026-02-02 18:17
- confirmed approach: keep odd-sized small aggregates by-value for C ABI; implement chunked handling in isel and update docs accordingly.

## 2026-02-02 13:35
- implemented chunked load/store helpers for odd-sized small aggregates and integrated them in x86_64 isel paths.
- tightened scratch register selection to avoid base/index/dest clobbers during chunked moves.
- documented odd-size small-aggregate by-value handling in `doc/masm/abi.md`.
- clang-format not available in this environment; formatting unchanged.

## 2026-02-02 13:39
- vreg stack layout audit: `scan_function` accounts for multi-slot vregs, but `alloc_vreg` always increments by 1, so vregs > 8 bytes can overlap adjacent slots.
- memory operand audit: index vregs are not resolved in `load_operand`/`emit_*`; only base vregs are handled, so indexed address modes can emit unresolved index regs.
- these findings need a confirmed problem statement and fix plan before code changes.

## 2026-02-02 19:43
- confirmed next steps: verify odd-size aggregate handling is complete in bootstrap, then mirror fixes to self-hosted isel/lowering and add self-hosted tests.
- clarified testing note: bootstrap has no unit tests; use the self-hosted test framework for new coverage.
## 2026-02-02 20:12
- added `PARITY_CHECKLIST.md` mapping bootstrap C files to self-hosted Mach equivalents with gap notes.
- bumped `src/commands/build.mach` loaded module cap from 64 → 256 (parity with bootstrap fix for `_start`).

## 2026-02-02 20:16
- replaced `src/commands/testing.mach` with full test harness (port of bootstrap cmd_test.c): AST transform for `test` blocks, harness generation, per-file compile + run.
- uses recursive file discovery under src/ and emits test binaries under out/<artifacts>/tests/.

## 2026-02-02 20:48
- implemented target-aware build in `src/commands/build.mach` (entrypoint + output path + module_path derived from entry file).
- run command now treats `target = "native"` as auto-select first target.

## 2026-02-02 20:52
- fixed self-hosted vreg slot allocation in `src/compiler/masm/isel.mach`: multi-slot vregs now reserve contiguous stack slots during scan; stack size derived from allocated slots.

## 2026-02-02 21:02
- added base/index vreg handling in `src/compiler/masm/isel.mach`: compute effective addresses via RCX/RAX with scale+offset; updated LOAD/STORE/LEA to use it.

## 2026-02-02 21:07
- implemented size-aware loads/stores in `src/compiler/masm/isel.mach` (vreg stack loads and mem load/store now emit byte/word/dword/qword ops).
- this enables odd-size aggregate copies to use byte-wise moves without corrupting higher bytes.

## 2026-02-02 23:52
- bootstrap x86_64 isel: resolve both base+index vregs for explicit mem operands (emit_mov/load/store/binary op), matching self-hosted fix.
- bootstrap lower: removed manual vreg bump for large return values; rely on alloc_vreg multi-slot sizing for platform-agnostic behavior.

## 2026-02-03 00:10
- normalized Mach syntax in tooling/helpers (removed ternaries + invalid inline blocks) across build/testing/dep/init/sema/isel.
- added `src/lang_test/odd_small_aggregates.mach` and updated helpers; tests compile but **all 10 cases crash at runtime**.
- next focus: odd-size small aggregate return/arg/copy paths in self-hosted masm lowering + isel (chunked load/store + ABI).
