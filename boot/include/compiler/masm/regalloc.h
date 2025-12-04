#ifndef MASM_REGALLOC_H
#define MASM_REGALLOC_H

#include "compiler/masm/isa/x86_64.h"
#include "compiler/masm/operand.h"
#include "compiler/masm/section.h"
#include <stdbool.h>
#include <stdint.h>

// scratch registers available for allocation (caller-saved)
// excludes: RSP, RBP (frame), RAX (return value, special ops)
// order prioritizes registers not used for arguments first
#define REGALLOC_SCRATCH_COUNT 9

typedef struct RegAlloc
{
    bool    in_use[MASM_X86_REG_COUNT]; // which registers are currently allocated
    int32_t spill_offset;               // next available spill slot (negative from RBP)
    int     spill_count;                // number of spilled values
} RegAlloc;

// initialize register allocator
void regalloc_init(RegAlloc *ra);

// allocate a scratch register, returns MASM_X86_REG_COUNT if none available
MasmX86Reg regalloc_alloc(RegAlloc *ra);

// allocate a specific register if available
bool regalloc_alloc_specific(RegAlloc *ra, MasmX86Reg reg);

// free a register
void regalloc_free(RegAlloc *ra, MasmX86Reg reg);

// check if a register is available
bool regalloc_is_available(RegAlloc *ra, MasmX86Reg reg);

// mark a register as in use (for pre-colored registers like function args)
void regalloc_reserve(RegAlloc *ra, MasmX86Reg reg);

// get a spill slot offset (allocates new slot)
int32_t regalloc_get_spill_slot(RegAlloc *ra);

// emit spill: save register to stack
void regalloc_emit_spill(RegAlloc *ra, MasmSection *text, MasmX86Reg reg, int32_t slot);

// emit reload: load register from stack
void regalloc_emit_reload(RegAlloc *ra, MasmSection *text, MasmX86Reg reg, int32_t slot);

// get the list of scratch registers in allocation order
const MasmX86Reg *regalloc_scratch_regs(void);

#endif // MASM_REGALLOC_H
