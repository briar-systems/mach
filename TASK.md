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
  - [x] Implement floating-point dispatch in `lower_binary_op`.

- [ ] **Phase 4: x86-64 Backend Hardening**
  - [x] **Integer Arithmetic Fixes**:
    - [x] Implement correct operand size handling for `DIV`/`REM` (8, 16, 32, 64-bit).
    - [x] Implement correct operand size handling for shifts (`SHL`, `SHR`, `SAR`).
    - [x] Ensure correct sign/zero extension when loading operands for these operations.
  - [ ] **Immediate Handling**:
    - [ ] Optimize immediate encodings (use 8-bit imm when possible).
    - [ ] Fix sign-extension logic for immediate operands.
  - [ ] **Correctness**:
    - [ ] Handle 3-operand to 2-operand conversion robustly (avoid clobbering src operands).
    - [ ] Implement x86-specific ABI handling (syscalls, calling convention, stack alignment).

- [ ] **Phase 5: Cleanup & Verification**
  - [ ] Remove obsolete x86-specific opcodes from the main header.
  - [ ] Remove old encoding logic from `isa/x86_64.c` (or merge into backend).
  - [x] Ensure `make cmach` builds successfully.
  - [ ] Verify all tests pass with `./out/bin/cmach test .`.

# Log
## [2024-05-20]
- Initialized TASK.md based on the architecture discussion and `doc/masm_spec.md`.

## [2024-05-21]
- Completed Phase 1: Defined portable IR opcodes in `ir.h`, updated `MasmOperand` to support types, and added helper functions for instruction creation.

## [2024-05-22]
- Completed Phase 2: Defined backend interface in `backend.h`, created x86-64 backend skeleton, and implemented basic translation logic using existing encoder.

## [2024-05-23]
- Progress on Phase 3:
  - Refactored `lower.c` to use virtual registers (`vreg`) instead of the old register allocator.
  - Implemented `lower_binary_op` for arithmetic, bitwise, and comparison operations using portable IR opcodes and type-aware selection.
  - Implemented `lower_assign` and variable access using `MASM_IR_STORE`, `MASM_IR_LOAD`, and `MASM_IR_LEA`.
  - Implemented short-circuit evaluation using portable branches.

## [2024-05-24]
- Completed Phase 3:
  - Finalized `lower.c` refactor, removing all legacy x86-specific code blocks and unused helper functions.
  - Implemented `lower_call` to emit `MASM_IR_CALL` and `MASM_IR_SYSCALL` with arguments marshaled into portable operands.
  - Cleaned up compilation errors in `lower.c` and `backend/x86_64.c`.
  - Verified `make cmach` builds successfully (Phase 5 item).
  - Implemented basic (naive) lowering for most `MASM_IR_*` opcodes in `backend/x86_64.c`.
  - Tests currently failing due to incorrect size handling in arithmetic operations.

## [2024-05-25]
- Addressed Integer Arithmetic Fixes in Phase 4:
  - Added support for 8, 16, 32, and 64-bit operand sizes in `emit_div_rem`, `emit_shift`, `emit_binary_op`, and `emit_cmp_branch`.
  - Implemented correct sign-extension logic for division using new helper opcodes `CBW`, `CWD`, `CDQ` (added to `instruction.h` and `isa/x86_64.c`).
  - Fixed encoding logic in `isa/x86_64.c` for byte/word operations in shifts and division.
  - Verified fixes with new test files `test_div.mach`, `test_shift.mach`, and `test_cmp.mach`, which cover signed/unsigned and various bit-width operations.