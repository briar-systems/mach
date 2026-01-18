#ifndef MASM_ABI_SYSV64_H
#define MASM_ABI_SYSV64_H

#include "compiler/masm/isa/x86_64/x86_64.h"

MasmX86Reg masm_sysv64_arg_reg(int index);
MasmX86Reg masm_sysv64_ret_reg(int index);

#endif
