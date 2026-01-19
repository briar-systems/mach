#ifndef MASM_ABI_SYSV64_H
#define MASM_ABI_SYSV64_H

#include <stdint.h>

uint32_t masm_sysv64_arg_reg(int index);
uint32_t masm_sysv64_ret_reg(int index);

#endif
