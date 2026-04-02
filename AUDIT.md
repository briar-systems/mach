# Mach Compiler Backend Audit Report

Comprehensive audit of the MIR-through-codegen pipeline by 6 independent expert reviewers reading every line of source.

---

## ARCHITECTURAL PROBLEMS (systemic, affect multiple subsystems)

### A1. Frame-slot optimization creates invisible vreg uses

**Files:** `isel.mach` sel_load/sel_store, `compile.mach` insert_alloca_uses, `regalloc.mach` extend_alloca_for_frame_access

`sel_load`/`sel_store` detect alloca pointers via `slot_offset()` and emit `[FP+offset]` directly, bypassing the alloca vreg. This makes the vreg's lifetime invisible to `build_ranges`. Two post-hoc workarounds (`insert_alloca_uses` and `extend_alloca_for_frame_access`) attempt to compensate by pattern-matching `[FP+offset]` accesses and injecting ISA_USE pseudo-instructions or extending ranges.

**Why this is wrong:** The workarounds are fragile — they rely on offset matching (which can collide if two allocas share an offset due to frame packing), they miss `src2` accesses, and they run after ISel when it's too late to fix the instruction stream. The correct approach is for the ISel to always emit an explicit vreg reference (either as the memory base or via an ISA_USE at emit time), eliminating both workarounds entirely.

### A2. Two-pass regalloc re-allocates all vregs from scratch

**Files:** `regalloc.mach` alloc_function, run_linear_scan_pass2, insert_spill_code

The split-range spill insertion rebuilds ALL ranges from the spill buffer and re-runs allocation. Original non-spilled vregs compete with fresh reload vregs for registers, reproducing the same pressure as pass 1. Result: 22,000+ reload vregs spilled in pass 2 for large functions.

Pre-coloring from pass 1 doesn't work because instruction positions shift after spill insertion — pass-1 range boundaries don't align with pass-2 positions. Two overlapping pass-1 ranges can map to non-overlapping pass-2 ranges (or vice versa), causing register conflicts.

**Why this is wrong:** Standard linear-scan allocators (LLVM, HotSpot) don't re-run allocation. They emit spill code during or immediately after the allocation pass, using the register that was just freed by the spill decision. The current architecture separates allocation from spill emission, making iterative refinement impossible without a full re-run.

### A3. Stack alignment violation in prologue

**Files:** `encode.mach` x86_emit_prologue, `compile.mach` compute_frame_size

The prologue emits: `PUSH RBP` (RSP -= 8), `MOV RBP, RSP`, `SUB RSP, total`. After PUSH, RSP is misaligned by 8. For RSP to be 16-byte aligned after SUB, `total` must be congruent to 8 mod 16. But `compute_frame_size` aligns `total` to 16, and the prologue adds `frame_size + callee_space` (both multiples of 8). The result: RSP is 16-byte aligned only by accident when callee_space is an odd multiple of 8.

**Impact:** Violates SysV AMD64 ABI. SSE/AVX instructions and external library calls that assume 16-byte stack alignment will fault.

### A4. SRET return convention is non-standard

**Files:** `lower/stmt.mach` lower_ret, `lower/expr.mach` emit_sret_call

SRET functions return the SRET pointer as the function result (via RAX). Standard SysV AMD64: SRET functions return void; the caller already knows where the result is because it passed the pointer. Returning the pointer is harmless on x86-64 (RAX is caller-saved) but violates the ABI and will break interop with C libraries or other compilers.

---

## CRITICAL BUGS (will produce wrong code)

### B1. Linker: sec_base_offsets persists across object files

**File:** `elf.mach:1160-1290`

`sec_base_offsets` is a local array in `parse_elf_object()`, initialized once per call. Each .o file gets its own call. **This is actually correct** — the array IS local to each parse call. However, `sec_map` is passed from the linker and reused. The real issue: when multiple .o files have sections with the same name (e.g., multiple `.data` sections), they're merged into the same output section. The `sec_base_offsets[shi]` for each .o correctly records where that .o's contribution starts within the merged section.

**Correction after deeper review:** The offset calculation is per-file. The ACTUAL linker bug is in symbol index mapping.

### B2. Linker: undefined symbol relocation resolves to wrong symbol

**File:** `elf.mach:1292`

```
relocs[idx].sym_idx = sym_base + (r_sym - 1)::i32;
```

When `r_sym = 0` (ELF undefined symbol), this produces `sym_base - 1`, pointing to the last symbol from the PREVIOUS .o file. Should special-case `r_sym == 0`.

### B3. String escape: hex escape loop desynchronization

**File:** `data.mach:104-151`

The length calculation loop for `\xHH` escapes advances `si += 4`, but the loop body also increments `out_len` unconditionally. In the second pass, the hex branch does `si += 4; di += 1; cnt;` which skips the outer `si += 1`, but the first pass doesn't have this same skip pattern. Missing bounds check: `si + 3 < text_len` for hex escapes.

### B4. Regalloc: MAX_ACTIVE overflow silently drops active entries

**File:** `regalloc.mach:600`

```
fun add_active(...) {
    if (@active_count >= MAX_ACTIVE) { ret; }
```

When the active set is full, new entries are silently dropped. The register is marked as not-free (by try_alloc_reg) but has no active entry, so expire_old can never free it. The register is permanently lost from the pool. After 4096 overflows, ALL registers are exhausted, forcing everything to spill.

### B5. Regalloc: scratch_reg2 not marked unavailable in gp_free

**File:** `regalloc.mach:940-943`

`is_reserved()` includes `scratch_reg2`, but `run_linear_scan` only sets `gp_free[scratch_reg] = false`, not `gp_free[scratch_reg2] = false`. The `try_alloc_reg` check `gp_free[ci] && !is_reserved(i, ci)` prevents assignment (is_reserved catches it), but the register stays "free" in the pool — it can never be allocated and never freed, wasting a slot.

### B6. Return type confusion in lower_ret

**File:** `lower/stmt.mach:229-234`

The return type is overwritten by the expression's type if it's an aggregate, regardless of the function's declared return type. If the function declares `-> u32` but returns a struct, SRET handling activates for a non-SRET function.

### B7. ELF sec_base_offsets buffer overflow

**File:** `elf.mach:1159-1208`

`sec_base_offsets` is `[256]usize`. If an .o file has > 256 section headers, writes overflow the stack array. No bounds check on `shi < 256`.

---

## HIGH-SEVERITY ISSUES (incorrect behavior in specific scenarios)

### H1. extend_alloca_for_frame_access misses src2

Only checks `src1` and `dst` for `[FP+offset]` accesses, not `src2`.

### H2. promote_single_store restricted to entry block

Rejects single-store allocas unless the store is in block 0, even when the store dominates all loads. Correct approach: check dominance.

### H3. compute_liveness iteration bound may be insufficient

`max_iter = bc * 2 + 4`. For chain-shaped CFGs of depth N, N iterations are needed minimum. Bound should be `bc * 4` or unbounded with the `changed` flag.

### H4. RegSet word_count truncation

`word_count` is `u16`. For `max_id > 4M`, the cast truncates, creating an undersized bitset. All liveness analysis breaks silently.

### H5. apply_vmap chain depth limit of 16

Transitive vmap resolution stops after 16 steps. Chains longer than 16 leave intermediate values, silently producing wrong SSA.

### H6. Missing SRET local falls through to wrong return path

If `$sret` local is missing (earlier error), `lower_ret` falls through to the register-return path for an aggregate, returning it in registers when the caller expects SRET.

### H7. Field array bounds not checked

`lower_field_ptr` indexes `actual_t.fields[field_idx]` without validating `field_idx < actual_t.field_count`.

---

## MEDIUM-SEVERITY ISSUES

### M1. Callee-save / spill slot gap

Callee-saves are placed at `[FP - (frame_size + 8*N)]`. Spill slots go up to `[FP - frame_size]`. The gap between them depends on alignment. Not a bug per se, but undocumented and fragile if frame_size computation changes.

### M2. No ABI abstraction in MIR lowering

`max_reg_ret = 16` is hardcoded. No per-target calling convention support. Parameter classification assumes linear indexing.

### M3. PC32 relocation not range-checked

`(sym_addr + addend - patch_addr)` is cast to `u32` without validating the result fits in signed 32 bits.

### M4. Entry point defaults silently to base_addr

If the entry symbol isn't found, no error is emitted. The binary runs but jumps to address 0x400000.

### M5. Relocation buffer overflow

`r.offset >= sec.size` check passes for an 8-byte relocation at `sec.size - 4`.

### M6. Section layout not sorted by type

Linker assigns vaddrs to sections in parsing order, not grouped by type (text, data, rodata, bss).

### M7. Encoder doesn't validate physical register IDs

Register IDs > 15 are silently truncated via `reg_bits(id) = id & 7`. A regalloc bug that leaks a vreg ID produces garbage encoding instead of an error.

### M8. REP MOVSB for struct copy

Performance issue. Modern compilers unroll or use MEMCPY.

---

## DESIGN DEBT (not bugs, but architectural weaknesses)

### D1. SRET pointer stored in unnecessary alloca

The SRET pointer parameter is stored into a freshly-allocated stack slot instead of being kept in a register. Adds memory traffic for every SRET return.

### D2. Type record mutation for memoized sizes

`type_size()` mutates the type record in place (`t.size = offset`). Not thread-safe.

### D3. No rematerialization or live-range splitting in regalloc

Spill decisions are all-or-nothing. A constant or cheap-to-recompute value gets the same spill treatment as an expensive one.

### D4. Mem2reg only promotes single-block and entry-block-single-store

No multi-block SSA construction with phi insertion. Most local variables in multi-block functions stay as alloca+load+store.

### D5. Parallel copy resolver uses a single fixed scratch register

`resolve_pcopy_group` uses `tgt.isa.scratch_reg` for cycle breaking. If that register is ever freed from the reserved set, cycles can't be broken.

---

## RECOMMENDED FIX ORDER

### Phase 1: Correctness (unblocks bootstrap)

1. **Fix stack alignment** — `total` in prologue should be `frame_size + callee_space`, adjusted so `(total + 8) % 16 == 0`
2. **Fix regalloc to not re-allocate in pass 2** — Pre-color pass-1 vregs as FIXED (skip allocation, only add to active for tracking). Only allocate fresh reload vregs.
3. **Fix MAX_ACTIVE overflow** — Return failure or dynamically resize
4. **Fix string escape hex bounds** — Check `si + 3 < text_len`
5. **Fix linker r_sym=0 handling** — Special-case undefined symbols
6. **Fix encoder vreg leak detection** — Assert register ID <= 15

### Phase 2: Architecture (eliminates fragile workarounds)

7. **Eliminate frame-slot optimization invisibility** — ISel should emit explicit vreg references for all alloca accesses. Remove `insert_alloca_uses` and `extend_alloca_for_frame_access`.
8. **Single-pass spill emission** — Emit spill code during allocation using the freed register, not in a separate phase.
9. **Fix SRET convention** — Return void from SRET functions per ABI spec.
10. **Fix stack alignment properly** — Account for PUSH RBP in frame size calculation.

### Phase 3: Completeness

11. Implement multi-block mem2reg with proper dominance/phi insertion
12. Add ABI abstraction to MIR lowering
13. Range-check PC32 relocations
14. Sort sections by type before vaddr assignment
15. Increase apply_vmap depth limit to MAX_PROMOTABLE
