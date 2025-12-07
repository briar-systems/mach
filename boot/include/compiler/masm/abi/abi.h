#ifndef MASM_ABI_H
#define MASM_ABI_H

#include <stdbool.h>
#include <stdint.h>
#include "compiler/masm/target.h"

// generic ABI query helpers (current implementation: sysv64 on x86_64)

uint8_t  masm_abi_pointer_size(MasmTarget target);
uint8_t  masm_abi_stack_align(MasmTarget target);
bool     masm_abi_has_red_zone(MasmTarget target);

uint8_t  masm_abi_int_arg_count(MasmTarget target);
uint32_t masm_abi_int_arg_reg(MasmTarget target, int index);

uint8_t  masm_abi_int_ret_count(MasmTarget target);
uint32_t masm_abi_int_ret_reg(MasmTarget target, int index);

// convenience helpers for common frame registers
uint32_t masm_target_stack_pointer_reg(MasmTarget target);
uint32_t masm_target_frame_pointer_reg(MasmTarget target);

#endif // MASM_ABI_H
