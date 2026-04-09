# Mach Compiler: Architectural Debt & Stability Report
**Date:** April 9, 2026
**Subject:** SEMA through Backend Pipeline Analysis

## 1. Executive Summary
The Mach compiler is currently suffering from systemic instability (frequent segfaults) and architectural "holes" in the backend. The primary causes are incomplete spill handling in the register allocator, pointer invalidation during MIR construction, and inconsistent handling of aggregate types. The pipeline lacks a formal ABI lowering phase, forcing the AST-to-MIR lowering to make fragile, target-specific decisions.

---

## 2. Detailed Findings

### A. Register Allocator: The "Spill Hole"
The register allocator's rewriting phase fails to handle virtual registers (VREGs) that have been spilled to the stack.

*   **Location:** `src/compiler/regalloc.mach` -> `rewrite_operand`
*   **Problem:** This function translates VREG IDs (>= 32) to physical registers (0-31). However, if a VREG is marked as `spilled`, the function simply ignores it. The resulting instruction stream still contains IDs >= 32.
*   **Impact:** **CRITICAL/SEGFAULT.** When the encoder attempts to use these IDs as indices into physical register arrays, it performs an out-of-bounds access.
*   **Solution:** Implement a multi-pass approach where spills are lowered into explicit `load` and `store` instructions *before* the final register assignment.

### B. CFG Inconsistency: Block Remapping
The block reordering pass breaks the link between SSA values and their defining blocks.

*   **Location:** `src/compiler/mir/compile.mach` -> `reorder_blocks_rpo`
*   **Problem:** This function reorders the `func.blocks` array to improve linear scan efficiency. It remaps branch targets in instructions, but it **does not** update the `block` field in `ir.Value`.
*   **Impact:** **CRITICAL.** Subsequent passes (like Liveness Analysis in `mir/cfg.mach`) that rely on `value.block` will use incorrect indices, leading to corrupted live ranges and incorrect register allocation.
*   **Solution:** Iterate through all `func.values` and update their `block` ID using the `remap` table during reordering.

### C. MIR Builder: Pointer Invalidation
The MIR construction API is prone to "dangling pointer" bugs.

*   **Location:** `src/compiler/mir/builder.mach` (e.g., `alloc_value`, `alloc_inst`)
*   **Problem:** The builder frequently takes pointers to elements within arrays that it then reallocates (e.g., `val block: *ir.Block = ?fb.func.blocks[idx]`). If a nested call triggers a `reallocate`, the original pointer becomes invalid.
*   **Impact:** **HIGH/SEGFAULT.** Intermittent crashes that are difficult to reproduce, especially in larger functions where array growth is frequent.
*   **Solution:** Replace direct pointers with index-based access or use a stable memory structure (like a linked list of pages/arenas) that does not move elements on growth.

### D. Lowering: The "8-Byte Split" Inconsistency
Aggregate types (structs/arrays) are handled inconsistently depending on their size.

*   **Location:** `src/compiler/mir/lower/expr.mach` -> `lower_ident`, `lower_field`, `lower_assign`
*   **Problem:** Aggregates <= 8 bytes are treated as "values" (loaded into registers), while aggregates > 8 bytes are treated as "pointers" (passed by reference). This logic is manually duplicated across every expression type.
*   **Impact:** **MEDIUM.** Extremely fragile code. A slight change in type size can cause a "Value vs. Pointer" mismatch between a caller and a callee, leading to stack corruption.
*   **Solution:** Implement a unified "Value Representation" strategy. All aggregates should be represented as pointers in the initial MIR, with a target-specific "ABI Lowering" pass deciding which ones to promote to registers.

---

## 3. Structural Design Flaws

### i. Lack of a Formal ABI Pass
The compiler currently performs ABI-specific tasks (like SRET for large returns and argument coercion) during AST-to-MIR lowering.
*   **Problem:** Targets like x86_64 and ARM64 have different rules for struct passing. Hardcoding these in `lower_expr.mach` makes the frontend target-dependent.
*   **Solution:** Introduce `src/compiler/mir/abi.mach`. This pass should run on the MIR to transform "High-Level MIR" (with abstract calls) into "Low-Level MIR" (with explicit SRET pointers and coerced registers).

### ii. SEMA Validation Gaps
*   **Problem:** `validate_sidetables` in `src/compiler/sema/check.mach` is commented out.
*   **Impact:** Incomplete or erroneous type information silently flows into the backend. Many "backend" segfaults are actually caused by the frontend providing `TYPE_NIL` for a node it failed to check.
*   **Solution:** Re-enable strict validation. If SEMA fails, the pipeline MUST stop before MIR lowering begins.

---

## 4. Implementation Roadmap

### Phase 1: Stabilization (The "No More Segfaults" Phase)
1.  **Fix Block Remapping**: Update `ir.Value.block` in `reorder_blocks_rpo`.
2.  **Fix Spill Rewrite**: Update `rewrite_operand` to panic or handle spilled vregs explicitly.
3.  **Audit Builder**: Change `FuncBuilder` to avoid holding pointers across reallocations.

### Phase 2: Refactoring (The "Snuff" Phase)
1.  **Unified Aggregate Lowering**: Remove the 8-byte special casing from `expr.mach`.
2.  **ABI Lowering Pass**: Implement a dedicated pass to handle calling conventions.
3.  **Spill Lowering Pass**: Move spill logic into a pre-regalloc MIR transformation.

### Phase 3: Performance & Scale
1.  **Linker Optimization**: Replace O(N^2) symbol lookup with a Hash Map in `linker.mach`.
2.  **Arena Allocation**: Implement a per-session or per-function Arena to eliminate manual `deallocate` calls and leaks.
