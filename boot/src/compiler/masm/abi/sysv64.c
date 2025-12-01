#include "compiler/masm/abi/sysv64.h"

static const MasmX86Reg ARG_REGS[] = {
    MASM_X86_RDI,
    MASM_X86_RSI,
    MASM_X86_RDX,
    MASM_X86_RCX,
    MASM_X86_R8,
    MASM_X86_R9
};

static const MasmX86Reg RET_REGS[] = {
    MASM_X86_RAX,
    MASM_X86_RDX
};

MasmX86Reg masm_sysv64_arg_reg(int index)
{
    if (index >= 0 && index < 6)
    {
        return ARG_REGS[index];
    }
    return MASM_X86_REG_COUNT;
}

MasmX86Reg masm_sysv64_ret_reg(int index)
{
    if (index >= 0 && index < 2)
    {
        return RET_REGS[index];
    }
    return MASM_X86_REG_COUNT;
}
