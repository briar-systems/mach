#ifndef MASM_ABI_SYSV64_H
#define MASM_ABI_SYSV64_H

#include "compiler/masm/isa/x86_64.h"

// sysv64 calling convention
// integer arguments: rdi, rsi, rdx, rcx, r8, r9
// return value: rax, rdx

MasmX86Reg masm_sysv64_arg_reg(int index);
MasmX86Reg masm_sysv64_ret_reg(int index);

#endif // MASM_ABI_SYSV64_H
