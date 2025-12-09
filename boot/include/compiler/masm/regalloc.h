#ifndef MASM_REGALLOC_H
#define MASM_REGALLOC_H

#include "compiler/masm/isa/spec.h"
#include "compiler/masm/operand.h"
#include "compiler/masm/section.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct RegAlloc
{
    bool       *in_use;          // dynamic array sized per isa
    uint32_t     reg_count;      // total registers for isa
    const uint32_t *scratch_regs; // allocation priority list
    uint8_t      scratch_count;
    const uint32_t *reserved_regs; // non-allocatable
    uint8_t      reserved_count;
    uint8_t      spill_slot_size; // bytes per spill slot
    uint32_t     fp_reg;         // frame pointer for spills
    int32_t      spill_offset;   // next available spill slot (negative from FP)
    int          spill_count;    // number of spilled values
} RegAlloc;

// initialize register allocator for the given ISA spec
void regalloc_init(RegAlloc *ra, const MasmISASpec *isa, uint32_t fp_reg, uint32_t sp_reg, uint8_t spill_slot_size);

// allocate a scratch register, returns UINT32_MAX if none available
uint32_t regalloc_alloc(RegAlloc *ra);

// allocate a specific register if available
bool regalloc_alloc_specific(RegAlloc *ra, uint32_t reg);

// free a register
void regalloc_free(RegAlloc *ra, uint32_t reg);

// check if a register is available
bool regalloc_is_available(RegAlloc *ra, uint32_t reg);

// mark a register as in use (for pre-colored registers like function args)
void regalloc_reserve(RegAlloc *ra, uint32_t reg);

// get a spill slot offset (allocates new slot)
int32_t regalloc_get_spill_slot(RegAlloc *ra);

// emit spill: save register to stack
void regalloc_emit_spill(RegAlloc *ra, MasmSection *text, uint32_t reg, int32_t slot);

// emit reload: load register from stack
void regalloc_emit_reload(RegAlloc *ra, MasmSection *text, uint32_t reg, int32_t slot);

// get the list of scratch registers in allocation order
const uint32_t *regalloc_scratch_regs(RegAlloc *ra);

#endif // MASM_REGALLOC_H
