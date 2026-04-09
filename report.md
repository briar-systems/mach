# Mach Compiler: Post-Fix Audit & Residual Debt Report
**Date:** April 9, 2026
**Subject:** Follow-up Analysis of SEMA through Backend Pipeline

## 1. Executive Summary
Following the recent integration of fixes, the Mach compiler has achieved significantly better stability. The most critical "segfault triggers"—specifically the **Register Allocator Spill Hole** and the **CFG Block Remapping Bug**—have been resolved. However, the project still contains significant architectural "debt" that will hinder long-term maintainability and performance at scale.

---

## 2. Resolved Issues (Verified)

### A. CFG Consistency: Value Block Remapping
*   **Status:** **FIXED**
*   **Location:** `src/compiler/mir/compile.mach` -> `reorder_blocks_rpo`
*   **Update:** The function now correctly iterates through `func.value_count` and updates the `block` field for every SSA value using the remapping table. This ensures that liveness analysis and register allocation receive accurate block definitions.

### B. Register Allocator: In-line Spill Handling
*   **Status:** **FIXED**
*   **Location:** `src/compiler/regalloc.mach` -> `alloc_function`
*   **Update:** The rewriting phase now explicitly checks for `spilled` virtual registers. It generates `load` instructions (reloads) before uses and `store` instructions (spills) after definitions using dynamically selected free registers. This eliminates the out-of-bounds access in the encoder.

---

## 3. Residual Architectural Debt

### A. MIR Builder: Persistent Pointer Invalidation
*   **Location:** `src/compiler/mir/builder.mach`
*   **Problem:** The builder still uses `std.allocator.reallocate` for blocks, instructions, and values.
*   **Risk:** **HIGH.** Callers often hold pointers to these elements (e.g., `val block: *ir.Block = ?fb.func.blocks[bi]`). If a subsequent call to `create_block` or `alloc_inst` triggers a reallocation, the pointer in the caller's stack frame becomes a "dangling pointer," leading to non-deterministic crashes.
*   **Solution:** Move to an index-based API (using `BlockId` instead of `*Block`) or implement a segmented arena/paged list for IR nodes to ensure pointer stability.

### B. Lowering: Hardcoded Aggregate Inconsistency
*   **Location:** `src/compiler/mir/lower/expr.mach`
*   **Problem:** The decision to treat a struct as a "value" (load into reg) vs. a "pointer" (alloca/pass-by-ref) is still hardcoded as a `size > 8` check throughout the lowering logic.
*   **Risk:** **MEDIUM.** This logic is duplicated across `lower_ident`, `lower_field`, `lower_assign`, and `lower_call`. It is target-dependent (e.g., some ABIs pass 16-byte structs in registers) and fragile.
*   **Solution:** Represent all aggregates as pointers in the initial MIR. Introduce a dedicated `mir_abi` pass to perform target-specific promotion of small aggregates to SSA registers.

### C. SEMA: Missing Tree-Walking Validation
*   **Location:** `src/compiler/sema/check.mach` -> `validate_sidetables`
*   **Problem:** The validation is still effectively a no-op.
*   **Context:** The comment correctly notes that index-range iteration produces false positives for dead/orphaned nodes (e.g., in `$if` blocks).
*   **Risk:** **MEDIUM.** Without this check, `TYPE_NIL` can flow into lowering, causing incorrect size calculations in the backend.
*   **Solution:** Implement a recursive tree-walking validator that starts from the module root and verifies every reachable expression node has a resolved type.

### D. Linker: O(N) Symbol Resolution
*   **Location:** `src/compiler/linker.mach` -> `find_symbol`
*   **Problem:** Symbol lookup remains a linear scan through the `symbols` array.
*   **Risk:** **LOW (Small Projects) / CRITICAL (Large Projects).** As project size increases (e.g., linking the standard library + user code), linking time will grow quadratically.
*   **Solution:** Implement a simple Hash Map for symbol resolution during the parse/merge phase.

---

## 4. Next Steps
While the immediate "segfault storm" has been quelled, the next priority should be the **MIR Builder stabilization** to eliminate the remaining pointer-invalidation risks, followed by a formal **ABI Lowering pass** to clean up the `lower_expr.mach` logic.
