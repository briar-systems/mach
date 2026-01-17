# Description
Refactor MASM (Mach Assembly) from an x86-centric instruction set to a minimal, portable Intermediate Representation (IR) based on Three-Operand Form (TOF). This change aims to support future backends (ARM64, RISC-V, WASM) while maintaining human readability and writability. The new IR abstracts away platform-specific details like condition flags and calling conventions, delegating them to platform-specific backends. See `doc/masm_spec.md` for the detailed specification.

# Outline
- [x] **Phase 1: Core Definitions**
  - [x] Create `boot/include/compiler/masm/ir.h` defining the new portable opcodes.
  - [x] Update `MasmInstruction` and `MasmOperand` structures to support the new IR requirements.
  - [x] Define helper macros/functions for creating IR instructions.

- [x] **Phase 2: Backend Interface & x86-64 Skeleton**
  - [x] Define a backend interface (vtable or function pointers) for code generation.
  - [x] Create `boot/src/compiler/masm/backend/x86_64.c` skeleton.
  - [x] Implement the translation logic for basic blocks and simple data movement.

- [x] **Phase 3: Frontend Lowering Update**
  - [x] Refactor `boot/src/compiler/masm/lower.c` to emit new portable IR opcodes.
  - [x] Replace x86-specific comparison/branch logic (SETcc, Jcc) with portable compare/branch ops.
  - [x] Update arithmetic and memory operations to use explicit signedness and TOF.
  - [x] Update function call and syscall lowering to use generic instructions.

- [ ] **Phase 4: x86-64 Backend Implementation**
  - [ ] Implement full instruction lowering in `backend/x86_64.c`.
  - [ ] Handle 3-operand to 2-operand conversion.
  - [ ] Re-implement immediate size checks and sign-extension fixes (from previous task).
  - [ ] Implement x86-specific ABI handling (syscalls, calling convention).

- [ ] **Phase 5: Cleanup & Verification**
  - [ ] Remove obsolete x86-specific opcodes from the main header.
  - [ ] Remove old encoding logic from `isa/x86_64.c` (or merge into backend).
  - [x] Ensure `make cmach` builds successfully.
  - [ ] Verify all tests pass with `./out/bin/cmach test .`.

Recommended Next Steps
1.  **Fix Floating Point Lowering**: Update `lower_binary_op` in `lower.c` to properly dispatch floating-point types to `MASM_IR_F*` opcodes.
2.  **Expand Backend Support**: Implement the missing opcodes in `backend/x86_64.c` (div/rem, shifts, floats) to stop the segfaults.
3.  **Debug Runtime**: Run a simple smoke test manually (verbose mode) to identify exactly where the naive backend generates invalid code (likely stack alignment or ABI issues).

# Log
## [Date]
- Initialized TASK.md based on the architecture discussion and `doc/masm_spec.md`.

## [Date]
- Completed Phase 1: Defined portable IR opcodes in `ir.h`, updated `MasmOperand` to support types, and added helper functions for instruction creation.

## [Date]
- Completed Phase 2: Defined backend interface in `backend.h`, created x86-64 backend skeleton, and implemented basic translation logic using existing encoder.

## [Date]
- Progress on Phase 3:
  - Refactored `lower.c` to use virtual registers (`vreg`) instead of the old register allocator.
  - Implemented `lower_binary_op` for arithmetic, bitwise, and comparison operations using portable IR opcodes and type-aware selection.
  - Implemented `lower_assign` and variable access using `MASM_IR_STORE`, `MASM_IR_LOAD`, and `MASM_IR_LEA`.
  - Implemented short-circuit evaluation using portable branches.

## [Date]
- Completed Phase 3:
  - Finalized `lower.c` refactor, removing all legacy x86-specific code blocks and unused helper functions.
  - Implemented `lower_call` to emit `MASM_IR_CALL` and `MASM_IR_SYSCALL` with arguments marshaled into portable operands.
  - Cleaned up compilation errors in `lower.c` and `backend/x86_64.c`.
  - Verified `make cmach` builds successfully (Phase 5 item).
- Work on Phase 4:
  - `backend/x86_64.c` now implements naive lowering for most `MASM_IR_*` opcodes, including arithmetic, bitwise, comparison/branching, and calls.
