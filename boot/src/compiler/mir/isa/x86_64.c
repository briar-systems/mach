#include "compiler/mir/isa/x86_64.h"
#include <string.h>

static const char *X86_64_REG_NAMES[] = {
    // gp registers
    "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
    // fp registers
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
};

const char *x86_64_reg_name(X86_64_Reg reg)
{
    if (reg >= X86_64_REG_COUNT)
    {
        return "unknown";
    }
    return X86_64_REG_NAMES[reg];
}

X86_64_Reg x86_64_reg_from_name(const char *name)
{
    if (!name)
    {
        return X86_64_REG_COUNT;
    }

    for (int i = 0; i < X86_64_REG_COUNT; i++)
    {
        if (strcmp(X86_64_REG_NAMES[i], name) == 0)
        {
            return (X86_64_Reg)i;
        }
    }

    return X86_64_REG_COUNT;
}

bool x86_64_reg_is_gp(X86_64_Reg reg)
{
    return reg >= X86_64_RAX && reg <= X86_64_R15;
}

bool x86_64_reg_is_fp(X86_64_Reg reg)
{
    return reg >= X86_64_XMM0 && reg <= X86_64_XMM15;
}

int x86_64_get_gp_reg_count()
{
    return 16; // rax..r15
}

int x86_64_get_fp_reg_count()
{
    return 16; // xmm0..xmm15
}

int x86_64_get_pointer_size()
{
    return 8; // 64-bit pointers
}
