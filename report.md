# Mach Compiler: Structural Audit & Ongoing Debt Report
**Date:** April 9, 2026
**Subject:** SEMA through Backend Pipeline - Review of Recent Fixes

## 1. Executive Summary
An audit of the compiler's pipeline was conducted to evaluate the recent fixes intended to address previously identified architectural debt. While immediate segfault triggers in the register allocator and CFG remapping have been resolved, the recent attempts to address structural flaws (like aggregate handling and SEMA validation) relied on **superficial band-aids rather than genuine architectural corrections**. The core systemic issues remain and must be resolved structurally.

---

## 2. Evaluation of Recent "Fixes"

### A. SEMA Validation: The "Band-Aid" Guard
*   **Location:** `src/compiler/mir/lower/expr.mach` & `src/compiler/sema/check.mach`
*   **The "Fix":** Instead of implementing a proper tree-walking validator in SEMA (`validate_sidetables`), a defensive guard was added directly inside `lower_expr` to return `ir.VALUE_NONE` if a node has `TYPE_NIL`.
*   **The Reality:** This is an architectural anti-pattern. The lowering phase should never be responsible for defensively checking if semantic analysis did its job. By silently dropping `TYPE_NIL` nodes, the compiler masks SEMA bugs, replacing segfaults with silently missing code generation or cascading errors downstream.
*   **Required Action:** Remove the defensive guards in `lower_expr.mach`. Implement a recursive tree-walking function in `validate_sidetables` that ensures every reachable expression in the AST has a valid type before the lowering phase begins. If SEMA fails, the pipeline must halt.

### B. Aggregate Lowering: Centralized Hacks
*   **Location:** `src/compiler/mir/lower/ctx.mach` (`load_value` / `store_value`)
*   **The "Fix":** The hardcoded `size > 8` checks were extracted from inline lowering code and centralized into `load_value` and `store_value` helper functions.
*   **The Reality:** Centralizing a hack does not fix the underlying architectural flaw. The AST-to-MIR lowering is still making target-specific ABI decisions (deciding if an aggregate fits in a register based on an arbitrary 8-byte limit). This forces target-dependent logic into a target-independent phase.
*   **Required Action:** Represent *all* aggregates as pointers (pass-by-reference/alloca) in the initial MIR generation. Introduce a distinct `mir_abi` pass (e.g., `src/compiler/mir/abi.mach`) that runs immediately prior to instruction selection. This pass should implement the specific calling convention (e.g., System V AMD64 ABI, AAPCS64) and promote small aggregates to registers according to target rules.

---

## 3. Unaddressed Architectural Debt

### C. MIR Builder: Persistent Pointer Invalidation
*   **Location:** `src/compiler/mir/builder.mach`
*   **Problem:** The `FuncBuilder` still manages memory by calling `allocator.reallocate` for arrays of `ir.Block`, `ir.Inst`, and `ir.Value`.
*   **Risk:** **HIGH.** Whenever `create_block` or `alloc_inst` requires growing these arrays, `reallocate` may move the array in memory. Any caller holding a raw pointer (e.g., `*ir.Block`) to an element in the old array will now hold a dangling pointer, leading to catastrophic and hard-to-reproduce memory corruption.
*   **Required Action:** Refactor the MIR Builder and its consumers to exclusively use opaque IDs (e.g., `BlockId`, `InstId`) for cross-references. Alternatively, change the underlying memory management to use an Arena or a paged list (where pages are never moved once allocated).

### D. Linker: O(N) Symbol Resolution
*   **Location:** `src/compiler/linker.mach` (`find_symbol`)
*   **Problem:** Symbol lookup remains a naive linear scan (`for (si < sym_count)`) over the `symbols` array.
*   **Risk:** **CRITICAL for Scale.** As project complexity grows, linking times will scale quadratically ($O(N^2)$ for resolving $N$ relocations against $N$ symbols), resulting in severe performance degradation.
*   **Required Action:** Introduce a Hash Map or, at minimum, sort the symbol table and use a binary search for symbol resolution during the linking phase.

---

## 4. Conclusion
The compiler is no longer immediately crashing on trivial cases, but the architecture is still fragile. We must move away from defensive "just-in-case" coding (like the `TYPE_NIL` guards) and focus on strict phase separation: SEMA proves correctness, Lowering generates naive MIR, ABI transforms for the target, and ISel generates instructions. The remaining items listed above must be fixed structurally.