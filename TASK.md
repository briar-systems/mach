# Description
Fix known compiler bugs in the Mach bootstrap compiler's MASM lowerer. The bugs affected comparisons, shifts, floating-point operations, and binary expressions with inline function calls.

# Outline
- [x] Bug #1: Signed right shift uses logical shift (SHR) instead of arithmetic shift (SAR)
  - [x] Add `MASM_OP_SAR` opcode to `boot/include/compiler/masm/instruction.h`
  - [x] Add encoding for SAR in `boot/src/compiler/masm/isa/x86_64.c`
  - [x] Modify `lower_expr` in `boot/src/compiler/masm/lower.c` to check if left operand type is signed
  - [x] Use `MASM_OP_SAR` for signed types, `MASM_OP_SHR` for unsigned types
  - [x] Verify test `operators: right shift signed` passes
- [x] Bug #2: Integer comparisons use wrong signedness opcodes
  - [x] Add unsigned comparison opcodes: `MASM_OP_SETB`, `MASM_OP_SETA`, `MASM_OP_SETBE`, `MASM_OP_SETAE`
  - [x] Add encoding for SETB/SETA/SETBE/SETAE in x86_64.c (0x92, 0x97, 0x96, 0x93)
  - [x] Modify comparison handling in `lower_expr` to check operand type signedness
  - [x] Use unsigned opcodes (SETB/SETA/etc.) for u8, u16, u32, u64, ptr types
  - [x] Use signed opcodes (SETL/SETG/etc.) for i8, i16, i32, i64 types
  - [x] Verify tests `comparisons: i32 negative vs positive` and `comparisons: u64 greater than large` pass
- [x] Bug #3: f64 comparisons use integer CMP instead of UCOMISD
  - [x] Add `MASM_OP_X86_UCOMISD` opcode to x86_64.h
  - [x] Add encoding for UCOMISD in x86_64.c (0x66 0x0F 0x2E /r)
  - [x] Add `MASM_OP_SETA` and `MASM_OP_SETAE` for unordered floating-point comparisons
  - [x] Modify `lower_expr` binary comparison handling:
    - [x] Detect when operands are f64 type using `type_is_fp_class()`
    - [x] Load operands into XMM registers using MOVQ
    - [x] Use UCOMISD for comparison instead of CMP
    - [x] Use correct condition codes for FP (SETA for >, SETAE for >=, SETE for ==, etc.)
  - [x] Verify all f64 comparison tests pass
- [x] Bug #4: Binary expressions with inline function calls produce wrong results
  - [x] Analyzed root cause: scratch registers are caller-saved and get clobbered by calls
  - [x] Added `expr_contains_call()` helper to detect if RHS contains any function call
  - [x] Modified binary expression lowering to force stack spill (PUSH/POP) when RHS contains a call
  - [x] Verify tests pass:
    - [x] `operators: bitwise or with cross-module inline call`
    - [x] `operators: multiply with inline recursive call`
    - [x] `operators: add with inline recursive calls`
- [x] Bug #5: f64 arithmetic not implemented (ADDSD/SUBSD/MULSD/DIVSD)
  - [x] Add SSE arithmetic opcodes to x86_64.h
  - [x] Add encoding for SSE arithmetic in x86_64.c (F2 0F 58/5C/59/5E)
  - [x] Detect float types in binary expression lowering
  - [x] Emit SSE instructions for f64 +, -, *, /
  - [x] Move result back to GPR as bit pattern
  - [x] Verify all f64 arithmetic tests pass
- [x] Bug #6: Cross-module method calls fail with "undefined field or method"
  - [x] Reproduce issue with `p.sum()` on imported type
  - [x] Fix in `boot/src/compiler/sema.c`: use resolved receiver type from method symbol instead of re-resolving AST
  - [x] Verify fix with local regression test
- [x] Task: Documentation updates
  - [x] Update `mach/doc/keywords.md` to clarify uninitialized variable behavior
  - [x] Create `mach/doc/proposals/type_conversions.md` for type widening/narrowing proposal
- [x] Feature: Support type resizing in casts (`::`)
  - [x] Modify `boot/src/compiler/sema.c` to allow casts between different sized types
  - [x] Modify `boot/src/compiler/masm/lower.c` to implement ZEXT/Trunc logic for casts
  - [x] Verify tests pass for widening (u8->u64, etc) and narrowing (u64->u8)
- [x] Run full test suite and verify 0 failures

# Summary
All bugs have been fixed. The test suite now passes with 323 tests and 0 failures.

Key changes:
1. Added `type_is_signed()` and `type_is_unsigned()` helpers for type classification
2. Added `expr_contains_call()` helper to detect calls in expression trees
3. Implemented proper signed vs unsigned comparison opcode selection
4. Implemented floating-point comparisons using UCOMISD with correct flag mapping
5. Implemented floating-point arithmetic using ADDSD/SUBSD/MULSD/DIVSD
6. Fixed register spill strategy to use stack when RHS contains function calls
7. Fixed cross-module method resolution by using canonical method types
8. Clarified variable initialization behavior in documentation
9. Implemented type resizing (ZEXT/Trunc) in explicit casts (`::`)

# Log

## 2025-01-17 (Initial)
- Initialized TASK.md for compiler bug fixes.
- Analyzed failing tests: 13 failures across 4 bug categories.
- Identified root causes in boot/src/compiler/masm/lower.c.

## 2025-01-17 (Session 2)
- Fixed signed right shift (SAR vs SHR)
- Fixed integer comparison signedness (SETB/SETA/SETBE/SETAE)
- Fixed f64 comparisons using UCOMISD
- Test count: 323, failures reduced from 17 to 14

## 2025-01-17 (Session 3)
- Fixed binary expressions with inline function calls:
  - Added `expr_contains_call()` helper
  - Force stack spill when RHS contains a call
  - Failures reduced from 17 to 14
- Implemented SSE floating-point arithmetic:
  - Added ADDSD/SUBSD/MULSD/DIVSD opcodes and encoding
  - Added float arithmetic detection and SSE instruction emission
  - All 14 remaining float tests now pass
- Final result: 323 tests, 0 failures

## 2025-01-17 (Session 4)
- Fixed cross-module method resolution issue in `sema.c`
- Updated documentation regarding variable initialization
- Drafted proposal for type conversion rules
- Implemented support for casts between types of different sizes (implicit ZEXT/Trunc) in `::` operator