#include "compiler/masm/regalloc.h"
#include "compiler/masm/instruction.h"
#include <string.h>

// scratch registers in allocation priority order
// avoid argument registers when possible to reduce conflicts
static const MasmX86Reg SCRATCH_REGS[REGALLOC_SCRATCH_COUNT] = {
    MASM_X86_R10, // caller-saved, not used for args
    MASM_X86_R11, // caller-saved, not used for args
    MASM_X86_RAX, // return value, but usable as scratch
    MASM_X86_RCX, // arg 4, but commonly free
    MASM_X86_RDX, // arg 3 / return value 2
    MASM_X86_RSI, // arg 2
    MASM_X86_RDI, // arg 1
    MASM_X86_R8,  // arg 5
    MASM_X86_R9,  // arg 6
};

void regalloc_init(RegAlloc *ra)
{
    memset(ra->in_use, 0, sizeof(ra->in_use));

    // reserve RSP and RBP - never allocate these
    ra->in_use[MASM_X86_RSP] = true;
    ra->in_use[MASM_X86_RBP] = true;

    // also reserve callee-saved registers for now (simpler implementation)
    // a more sophisticated allocator would save/restore these
    ra->in_use[MASM_X86_RBX] = true;
    ra->in_use[MASM_X86_R12] = true;
    ra->in_use[MASM_X86_R13] = true;
    ra->in_use[MASM_X86_R14] = true;
    ra->in_use[MASM_X86_R15] = true;

    ra->spill_offset = 0;
    ra->spill_count  = 0;
}

MasmX86Reg regalloc_alloc(RegAlloc *ra)
{
    for (int i = 0; i < REGALLOC_SCRATCH_COUNT; i++)
    {
        MasmX86Reg reg = SCRATCH_REGS[i];
        if (!ra->in_use[reg])
        {
            ra->in_use[reg] = true;
            return reg;
        }
    }
    return MASM_X86_REG_COUNT; // none available
}

bool regalloc_alloc_specific(RegAlloc *ra, MasmX86Reg reg)
{
    if (reg >= MASM_X86_REG_COUNT)
    {
        return false;
    }
    if (ra->in_use[reg])
    {
        return false;
    }
    ra->in_use[reg] = true;
    return true;
}

void regalloc_free(RegAlloc *ra, MasmX86Reg reg)
{
    if (reg < MASM_X86_REG_COUNT)
    {
        // don't free reserved registers
        if (reg != MASM_X86_RSP && reg != MASM_X86_RBP && reg != MASM_X86_RBX && reg != MASM_X86_R12 && reg != MASM_X86_R13 && reg != MASM_X86_R14 && reg != MASM_X86_R15)
        {
            ra->in_use[reg] = false;
        }
    }
}

bool regalloc_is_available(RegAlloc *ra, MasmX86Reg reg)
{
    if (reg >= MASM_X86_REG_COUNT)
    {
        return false;
    }
    return !ra->in_use[reg];
}

void regalloc_reserve(RegAlloc *ra, MasmX86Reg reg)
{
    if (reg < MASM_X86_REG_COUNT)
    {
        ra->in_use[reg] = true;
    }
}

int32_t regalloc_get_spill_slot(RegAlloc *ra)
{
    ra->spill_offset -= 8; // each spill slot is 8 bytes
    ra->spill_count++;
    return ra->spill_offset;
}

void regalloc_emit_spill(RegAlloc *ra, MasmSection *text, MasmX86Reg reg, int32_t slot)
{
    (void)ra;
    MasmOperand src = masm_operand_register(reg, 8);
    MasmOperand dst = masm_operand_memory_simple(MASM_X86_RBP, slot, 8);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
}

void regalloc_emit_reload(RegAlloc *ra, MasmSection *text, MasmX86Reg reg, int32_t slot)
{
    (void)ra;
    MasmOperand dst = masm_operand_register(reg, 8);
    MasmOperand src = masm_operand_memory_simple(MASM_X86_RBP, slot, 8);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
}

const MasmX86Reg *regalloc_scratch_regs(void)
{
    return SCRATCH_REGS;
}
