#include "compiler/masm/regalloc.h"
#include "compiler/masm/instruction.h"
#include <stdlib.h>

static bool is_reserved(RegAlloc *ra, uint32_t reg)
{
    for (uint8_t i = 0; i < ra->reserved_count; i++)
    {
        if (ra->reserved_regs[i] == reg) return true;
    }
    return false;
}

void regalloc_init(RegAlloc *ra, const MasmISASpec *isa, uint32_t fp_reg, uint32_t sp_reg, uint8_t spill_slot_size)
{
    ra->reg_count      = isa ? isa->reg_count : 0;
    ra->scratch_regs   = isa ? isa->scratch_regs : NULL;
    ra->scratch_count  = isa ? isa->scratch_count : 0;
    ra->reserved_regs  = isa ? isa->reserved_regs : NULL;
    ra->reserved_count = isa ? isa->reserved_count : 0;
    ra->spill_slot_size = spill_slot_size ? spill_slot_size : 8;
    ra->fp_reg         = fp_reg;
    ra->spill_offset   = 0;
    ra->spill_count    = 0;

    if (ra->reg_count == 0)
    {
        ra->in_use = NULL;
        return;
    }

    ra->in_use = calloc(ra->reg_count, sizeof(bool));

    // reserve registers listed by ISA (e.g., sp/fp, callee-saved)
    for (uint8_t i = 0; i < ra->reserved_count; i++)
    {
        uint32_t reg = ra->reserved_regs[i];
        if (reg < ra->reg_count)
        {
            ra->in_use[reg] = true;
        }
    }

    // always reserve frame pointer passed in
    if (fp_reg < ra->reg_count)
    {
        ra->in_use[fp_reg] = true;
    }
    if (sp_reg < ra->reg_count)
    {
        ra->in_use[sp_reg] = true;
    }
}

uint32_t regalloc_alloc(RegAlloc *ra)
{
    for (uint8_t i = 0; i < ra->scratch_count; i++)
    {
        uint32_t reg = ra->scratch_regs[i];
        if (reg < ra->reg_count && !ra->in_use[reg])
        {
            ra->in_use[reg] = true;
            return reg;
        }
    }
    return UINT32_MAX; // none available
}

bool regalloc_alloc_specific(RegAlloc *ra, uint32_t reg)
{
    if (reg >= ra->reg_count)
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

void regalloc_free(RegAlloc *ra, uint32_t reg)
{
    if (reg < ra->reg_count)
    {
        // don't free reserved registers
        if (!is_reserved(ra, reg) && reg != ra->fp_reg)
        {
            ra->in_use[reg] = false;
        }
    }
}

bool regalloc_is_available(RegAlloc *ra, uint32_t reg)
{
    if (reg >= ra->reg_count)
    {
        return false;
    }
    return !ra->in_use[reg];
}

void regalloc_reserve(RegAlloc *ra, uint32_t reg)
{
    if (reg < ra->reg_count)
    {
        ra->in_use[reg] = true;
    }
}

int32_t regalloc_get_spill_slot(RegAlloc *ra)
{
    ra->spill_offset -= ra->spill_slot_size ? ra->spill_slot_size : 8;
    ra->spill_count++;
    return ra->spill_offset;
}

void regalloc_emit_spill(RegAlloc *ra, MasmSection *text, uint32_t reg, int32_t slot)
{
    MasmOperand src = masm_operand_register(reg, ra->spill_slot_size);
    MasmOperand dst = masm_operand_memory_simple(ra->fp_reg, slot, ra->spill_slot_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
}

void regalloc_emit_reload(RegAlloc *ra, MasmSection *text, uint32_t reg, int32_t slot)
{
    MasmOperand dst = masm_operand_register(reg, ra->spill_slot_size);
    MasmOperand src = masm_operand_memory_simple(ra->fp_reg, slot, ra->spill_slot_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
}

const uint32_t *regalloc_scratch_regs(RegAlloc *ra)
{
    return ra->scratch_regs;
}
