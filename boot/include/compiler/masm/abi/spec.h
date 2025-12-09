#ifndef MASM_ABI_SPEC_H
#define MASM_ABI_SPEC_H

#include <stdbool.h>
#include <stdint.h>
#include "compiler/masm/target.h"

typedef struct MasmABISpec
{
    uint8_t  pointer_size;
    uint8_t  stack_align;
    bool     has_red_zone;

    const uint32_t *int_arg_regs;
    uint8_t        int_arg_count;

    const uint32_t *int_ret_regs;
    uint8_t        int_ret_count;
} MasmABISpec;

// select ABI spec for a target; returns NULL if unsupported
const MasmABISpec *masm_abi_spec_select(MasmTarget target);

// compatibility helpers (backed by the selected ABI spec)
uint8_t  masm_abi_pointer_size(MasmTarget target);
uint8_t  masm_abi_stack_align(MasmTarget target);
bool     masm_abi_has_red_zone(MasmTarget target);
uint8_t  masm_abi_int_arg_count(MasmTarget target);
uint32_t masm_abi_int_arg_reg(MasmTarget target, int index);
uint8_t  masm_abi_int_ret_count(MasmTarget target);
uint32_t masm_abi_int_ret_reg(MasmTarget target, int index);

#endif // MASM_ABI_SPEC_H
