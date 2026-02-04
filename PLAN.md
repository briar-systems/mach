# Self-Hosted Compiler Completion Plan

**Status:** In Progress - Phase 2: 3-Layer Architecture Refactor
**Date:** 2026-02-04
**Architecture Target:** AST → IR → x86_64 Opcodes → Bytes

## Overview

This document tracks the completion roadmap for the Mach self-hosted compiler to achieve full parity with the bootstrap compiler and enable future multi-target support.

## Architecture Philosophy

MASM (Mach Assembly) is a **three-layer intermediate representation** designed for portability across architectures while maintaining efficiency.

```
Layer 1: Portable IR (48 opcodes, three-operand, flag-free, virtual registers)
    ↓
Layer 2: Target-Specific Opcodes (x86_64, ARM64, RISC-V, etc.)
    ↓
Layer 3: Machine Code Bytes
```

### Design Principles

1. **Portability First**: IR must be target-agnostic. No x86-specific code in lower.mach.
2. **Clean Separation**: Each layer has distinct responsibilities:
   - Lowerer: AST semantics → IR
   - isel: IR → target opcodes
   - Encoder: Target opcodes → bytes
3. **Generic Opcodes**: MASM opcodes represent intent, not specific instruction encodings.
4. **Modularity**: Clear boundaries enable future ISA additions (ARM64, RISC-V) without touching lowerer.

## Phase Breakdown

### Phase 1: Critical Bug Fix (SKIPPED - addressed in Phase 2 refactor)

**Rationale:** The syscall DSL bug stems from the 2-layer architecture mixing concerns. Refactoring to 3-layer will inherently fix this by proper separation.

### Phase 2: 3-Layer Architecture Refactor (CURRENT)

**Objective:** Transform from 2-layer (AST → IR → Bytes) to 3-layer (AST → IR → Opcodes → Bytes)

#### 2a: Create x86_64 Opcode Layer

**File:** `src/compiler/masm/isa/x86_64/opcode.mach`

Defines x86_64 instruction set as distinct opcodes. Generic design:
- `MOV` (not MOV_RR/MOV_RI/MOV_RM variants)
- `ADD`, `SUB`, `MUL`, `DIV`
- `AND`, `OR`, `XOR`, `NOT`
- `SHL`, `SHR`, `SAR`
- `CMP`, `TEST`
- `JMP`, `Jcc` family, `CALL`, `RET`
- `PUSH`, `POP`, `LEA`
- `SYSCALL`, `HLT`
- Floating point: `FADD`, `FSUB`, `FMUL`, `FDIV`, `FCMP`

**Operand System:** Each opcode carries operand type info (reg-reg, reg-imm, reg-mem, etc.)

#### 2b: Refactor Inline Assembly Parser

**File:** `src/compiler/masm/isa/x86_64/parse.mach`

Extract from current `x86asm.mach`:
- Intel syntax parsing (full support)
- Register name → ID mapping
- Operand parsing (reg, imm, mem, label)
- Mnemonic recognition
- **Key Change:** Emit x86_64 opcodes + operands (NOT bytes!)

#### 2c: Create Encoder

**File:** `src/compiler/masm/isa/x86_64/encode.mach`

Responsibility: Convert x86_64 opcodes to machine code bytes

**Implementation:**
- REX prefix calculation
- ModR/M encoding
- SIB byte generation
- Immediate encoding (8/16/32/64-bit)
- Displacement handling

**Separation:** Encoder knows nothing about IR or Mach AST. Pure x86_64 → bytes.

#### 2d: Refactor isel.mach

**Current:** IR → bytes (mixed concerns)
**Target:** IR → x86_64 opcodes

**Changes:**
- `select_mov()` → emit `MOV` opcode with appropriate operands
- `select_add()` → emit `ADD` opcode
- `select_syscall()` → emit `SYSCALL` opcode
- All emit functions create x86_64 instructions, append to section

**Section Structure:**
- Collect x86_64 instructions during isel
- Run encoder as final pass over each section

### Phase 3: Feature Completion

#### 3a: Floating Point

**IR additions:**
- `ir_fadd`, `ir_fsub`, `ir_fmul`, `ir_fdiv`, `ir_fcmp`
- `ir_fmov`, `ir_fconv` (conversion between float sizes)

**x86_64 opcodes:**
- SSE: `FADD`, `FSUB`, `FMUL`, `FDIV`, `FCMP`, `FMOV`

**Encoder:** SSE instruction encoding (VEX/legacy prefixes, XMM registers)

**Status:** Enable floats.mach test module

#### 3b: Optimization Framework

**File:** `src/compiler/masm/opt/pass.mach`

Skeleton infrastructure for future passes:

```mach
pub rec OptimizationPass {
    name: str;
    run: fun(*masm.Masm) bool;
}

pub fun register_pass(pass: OptimizationPass);
pub fun run_passes(m: *masm.Masm);
```

**Future passes:**
- Peephole (dead stores, redundant mov, push/pop pairs)
- Constant folding
- Strength reduction
- Register allocation improvements

#### 3c: Build System Features

**Missing features:**
- `-I` flag for module prefix mapping
- Library/archive output mode (.a files)
- Single-file with imports support

**Impact:** Unblocks real-world usage beyond test suite

#### 3d: Test Runner Completeness

**Missing features:**
- `--verbose` flag (show all results, not just failures)
- `--modules` flag (module-level progress)
- Per-test process isolation

### Phase 4: Verification

**Success Criteria:**
- [ ] Full test suite passes: `./out/bin/imach test .`
- [ ] Self-compilation works: imach can compile itself
- [ ] Binary correctness: Output matches cmach byte-for-byte (or close)
- [ ] Architecture documented: Clear separation of layers demonstrated

## File Structure Target

```
src/compiler/masm/
├── ir.mach              # Layer 1: Portable IR (exists)
├── lower.mach           # Layer 1: AST → IR (exists, refactor DSL)
├── isel.mach            # Layer 2: IR → Target Opcodes (refactor)
├── isa/
│   ├── spec.mach        # ISA abstraction layer
│   └── x86_64/
│       ├── spec.mach    # x86_64 ISA spec (exists)
│       ├── opcode.mach  # Layer 2: x86_64 opcodes (NEW)
│       ├── parse.mach   # Layer 2: Inline asm parser (NEW)
│       └── encode.mach  # Layer 3: x86_64 → bytes (NEW)
├── abi/
│   └── sysv64.mach      # Calling convention (exists)
├── opt/
│   └── pass.mach        # Optimization framework (NEW)
└── of/
    └── elf.mach         # Object file output (exists)
```

## Design Decisions

### 1. Opcode Granularity
**Decision:** Keep MASM opcodes generic
- `MOV` with operand type metadata, not `MOV_RR`/`MOV_RI`/`MOV_RM`
- More flexible, easier to add new addressing modes
- Bootstrap's granular approach was for simplicity; we want extensibility

### 2. Parser Scope
**Decision:** Support full Intel syntax
- Parser should handle complete x86_64 instruction set
- Not just the ~30 instructions bootstrap supports
- Future-proofing for real-world inline assembly needs

### 3. Modularity
**Decision:** Segregate at natural boundaries
- `isel.mach` → target opcodes (no encoding knowledge)
- `encode.mach` → bytes (pure encoding logic)
- `parse.mach` → Intel syntax → opcodes (no emission)
- Clean interfaces enable testing individual layers

### 4. Bug Strategy
**Decision:** Refactor first, fix second
- The syscall DSL bug stems from mixed concerns in 2-layer architecture
- Proper 3-layer separation will inherently fix the issue
- If not, the clean architecture makes debugging trivial

## Current Status

**Phase:** 2a - Creating x86_64 opcode layer
**Next Action:** Define opcode.mach with generic x86_64 instruction set

## Notes

- Target: x86_64 Linux only for MVP
- ARM64/RISC-V support added later by implementing:
  - `src/compiler/masm/isa/arm64/` (spec, opcode, parse, encode)
  - `src/compiler/masm/abi/aarch64/` (calling convention)
- No changes needed to lower.mach for new ISAs

## References

- Bootstrap reference: `boot/src/compiler/masm/isa/x86_64/` (isel.c, encode.c, asm.c)
- IR design: `boot/include/compiler/masm/ir.h` and `src/compiler/masm/ir.mach`
- ABI spec: `src/compiler/masm/abi/sysv64.mach`
