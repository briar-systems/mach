# Phase 2 Progress Update - 3-Layer Architecture Refactor

## Completed ✅

### 1. Created x86_64 Opcode Layer
**File:** `src/compiler/masm/isa/x86_64/opcode.mach`

- **164 generic opcodes** covering complete x86_64 instruction set:
  - Data movement: MOV, MOVSX, MOVZX, LEA, PUSH, POP, etc.
  - Arithmetic: ADD, SUB, MUL, IMUL, DIV, IDIV, etc.
  - Bitwise: AND, OR, XOR, SHL, SHR, SAR, BT, BSF, etc.
  - Comparison: CMP, TEST, full SETcc family
  - Control flow: JMP, CALL, RET, full Jcc family, CMOVcc
  - Floating point: complete SSE instruction set (ADDSS/SD, SUBSS/SD, etc.)
  - System: SYSCALL, INT, CPUID, RDTSC, etc.
  - Atomic: XCHG, CMPXCHG, XADD
  - String: MOVS, STOS, LODS
  - Fences: LFENCE, SFENCE, MFENCE

- **Design:** Generic opcodes with operand types carrying addressing mode info
- **Helper functions:** get_name(), get_category() for debugging

### 2. Started Encoder Layer
**File:** `src/compiler/masm/isa/x86_64/encode.mach` (partial)

**Implemented:**
- REX prefix encoding helpers
- ModR/M encoding
- SIB encoding

**Still needed:**
- Instruction-specific encoding functions for each opcode category
- Main encode_instruction() dispatcher
- Integration with spec.mach

## Next Steps 🚧

### Immediate Tasks

1. **Complete encode.mach** (~500-800 lines needed)
   - Encode functions for each instruction category
   - Handle REX, ModR/M, SIB, displacement, immediates
   - Pattern: Bootstrap's encode.c as reference

2. **Refactor isel.mach** 
   - Change from emitting bytes to emitting x86_64 opcodes
   - Create instructions with opcodes + operands
   - Append to section's instruction list (not bytes)

3. **Create parse.mach** (inline assembly parser)
   - Extract from x86asm.mach
   - Parse Intel syntax → x86_64 opcodes (not bytes)
   - Support full x86_64 instruction set

4. **Update spec.mach**
   - Implement encode() to dispatch to x86_64 encoder
   - Wire up the 3-layer pipeline

5. **Build and Test**
   - Ensure everything compiles
   - Run tests to verify functionality
   - Debug any issues

## Architecture Reminder

```
Mach AST → IR (48 opcodes) → x86_64 Opcodes (164) → Bytes
    ↓           ↓                 ↓                  ↓
 lower.mach  isel.mach        encode.mach        section
         
Key Principles:
- Generic opcodes (not MOV_RR/MOV_RI/MOV_RM)
- Operand types carry addressing mode info
- Clean separation: IR is target-agnostic
- Encoder knows nothing about Mach semantics
```

## Files Modified/Created

**New files:**
- `src/compiler/masm/isa/x86_64/opcode.mach` ✅ (complete)
- `src/compiler/masm/isa/x86_64/encode.mach` 🚧 (partial)

**To be modified:**
- `src/compiler/masm/isel.mach` (refactor to emit opcodes)
- `src/compiler/masm/isa/x86_64/parse.mach` (extract from x86asm.mach)
- `src/compiler/masm/isa/spec.mach` (wire up encoder)

## Bootstrap Reference

For encoding details, reference:
- `boot/src/compiler/masm/isa/x86_64/encode.c` (complete encoding logic)
- `boot/src/compiler/masm/isa/x86_64/isel.c` (IR to x86_64 opcodes)
- `boot/src/compiler/masm/isa/x86_64/asm.c` (inline asm parsing)

## Notes

- Encoder implementation is complex (~800 lines)
- Recommend using bootstrap encode.c as direct reference
- Each instruction family needs specific encoding logic
- Some instructions need special handling (e.g., MUL/DIV use implicit operands)
