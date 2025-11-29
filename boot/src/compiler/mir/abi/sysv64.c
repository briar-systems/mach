#include "compiler/mir/abi/sysv64.h"
#include "compiler/mir/isa/x86_64.h"

int sysv64_get_stack_alignment()
{
    return 16; // 16-byte stack alignment
}

int sysv64_get_shadow_space_size()
{
    return 0; // no shadow space (windows-only feature)
}

int sysv64_get_int_arg_regs(int *regs, int max_count)
{
    // sysv64 integer argument registers: rdi, rsi, rdx, rcx, r8, r9
    static const int arg_regs[] = {
        X86_64_RDI,
        X86_64_RSI,
        X86_64_RDX,
        X86_64_RCX,
        X86_64_R8,
        X86_64_R9,
    };

    int count = sizeof(arg_regs) / sizeof(arg_regs[0]);
    if (count > max_count)
    {
        count = max_count;
    }

    for (int i = 0; i < count; i++)
    {
        regs[i] = arg_regs[i];
    }

    return count;
}

int sysv64_get_fp_arg_regs(int *regs, int max_count)
{
    // sysv64 floating point argument registers: xmm0..xmm7
    static const int arg_regs[] = {
        X86_64_XMM0, X86_64_XMM1, X86_64_XMM2, X86_64_XMM3,
        X86_64_XMM4, X86_64_XMM5, X86_64_XMM6, X86_64_XMM7,
    };

    int count = sizeof(arg_regs) / sizeof(arg_regs[0]);
    if (count > max_count)
    {
        count = max_count;
    }

    for (int i = 0; i < count; i++)
    {
        regs[i] = arg_regs[i];
    }

    return count;
}

int sysv64_get_int_ret_regs(int *regs, int max_count)
{
    // sysv64 integer return registers: rax, rdx
    static const int ret_regs[] = {X86_64_RAX, X86_64_RDX};

    int count = sizeof(ret_regs) / sizeof(ret_regs[0]);
    if (count > max_count)
    {
        count = max_count;
    }

    for (int i = 0; i < count; i++)
    {
        regs[i] = ret_regs[i];
    }

    return count;
}

int sysv64_get_fp_ret_regs(int *regs, int max_count)
{
    // sysv64 floating point return registers: xmm0, xmm1
    static const int ret_regs[] = {X86_64_XMM0, X86_64_XMM1};

    int count = sizeof(ret_regs) / sizeof(ret_regs[0]);
    if (count > max_count)
    {
        count = max_count;
    }

    for (int i = 0; i < count; i++)
    {
        regs[i] = ret_regs[i];
    }

    return count;
}

bool sysv64_is_caller_saved(int physical_reg_id)
{
    // caller-saved (volatile): rax, rcx, rdx, rsi, rdi, r8-r11, xmm0-xmm15
    return physical_reg_id == X86_64_RAX ||
           physical_reg_id == X86_64_RCX ||
           physical_reg_id == X86_64_RDX ||
           physical_reg_id == X86_64_RSI ||
           physical_reg_id == X86_64_RDI ||
           (physical_reg_id >= X86_64_R8 && physical_reg_id <= X86_64_R11) ||
           (physical_reg_id >= X86_64_XMM0 && physical_reg_id <= X86_64_XMM15);
}

bool sysv64_is_callee_saved(int physical_reg_id)
{
    // callee-saved (non-volatile): rbx, rbp, r12-r15
    return physical_reg_id == X86_64_RBX ||
           physical_reg_id == X86_64_RBP ||
           (physical_reg_id >= X86_64_R12 && physical_reg_id <= X86_64_R15);
}

int sysv64_classify_param(Type *type)
{
    if (!type)
    {
        return 1; // default: single register
    }

    // primitives and pointers: single register
    if (type->size <= 8)
    {
        return 1;
    }

    // structs > 16 bytes: passed by reference (return 0)
    if (type->kind == TYPE_STRUCT && type->size > 16)
    {
        return 0;
    }

    // structs 9-16 bytes: split across 2 registers
    if (type->kind == TYPE_STRUCT && type->size > 8 && type->size <= 16)
    {
        return 2;
    }

    // default: single register
    return 1;
}

int sysv64_classify_return(Type *type)
{
    if (!type)
    {
        return 0; // void
    }

    // primitives and pointers: single register
    if (type->size <= 8)
    {
        return 1;
    }

    // structs <= 16 bytes: up to 2 registers (rax, rdx)
    if (type->kind == TYPE_STRUCT && type->size <= 16)
    {
        return (type->size > 8) ? 2 : 1;
    }

    // structs > 16 bytes: return via hidden pointer (return 0)
    if (type->kind == TYPE_STRUCT && type->size > 16)
    {
        return 0;
    }

    // default: single register
    return 1;
}
