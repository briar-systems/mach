#ifndef MASM_ISA_SPEC_H
#define MASM_ISA_SPEC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "compiler/masm/instruction.h"
#include "compiler/masm/operand.h"
#include "compiler/masm/target.h"

struct Masm;

// ISA-level opcode provider for target-specific operations
typedef struct MasmISASpec
{
    // register roles
    MasmOperand (*reg_result)(uint8_t size);
    MasmOperand (*reg_tmp0)(uint8_t size);
    MasmOperand (*reg_tmp1)(uint8_t size);
    // Adding div_hi and div_lo roles for division operations
    MasmOperand (*reg_div_hi)(uint8_t size);
    MasmOperand (*reg_div_lo)(uint8_t size);
    MasmOperand (*reg_arg)(int index, uint8_t size); // integer args per ABI/ISA
    MasmOperand (*reg_sp)(uint8_t size);
    MasmOperand (*reg_fp)(uint8_t size);

    // register set metadata
    uint32_t       reg_count;       // total registers for ISA
    const uint32_t *scratch_regs;   // allocation order
    uint8_t        scratch_count;
    const uint32_t *reserved_regs;  // permanently reserved (sp/fp/callee-saved)
    uint8_t        reserved_count;

    // opcode hooks
    uint32_t (*op_syscall)();

    // inline asm / operand parsing hook
    MasmOperand (*parse_reg)(const char *name, uint8_t ptr_size);

    // instruction selection: lower IR to target-specific opcodes
    void (*isel)(struct Masm *masm);

    // encode instruction to bytes, returns byte count
    int (*encode)(MasmInstruction inst, uint8_t *buffer, size_t size);
}
MasmISASpec;

// select ISA spec for a target; returns NULL if unsupported
const MasmISASpec *masm_isa_spec_select(MasmTarget target);

#endif // MASM_ISA_SPEC_H
