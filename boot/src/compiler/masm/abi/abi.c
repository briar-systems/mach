#include "compiler/masm/abi/abi.h"
#include "compiler/masm/abi/sysv64.h"
#include "compiler/masm/isa/x86_64.h"

// NOTE: Only sysv64 on x86_64 is implemented right now. Additional
// targets can extend these switches without changing callers.

uint8_t masm_abi_pointer_size(MasmTarget target)
{
    switch (target.isa)
    {
    case MASM_ISA_X86_64:
        return 8;
    default:
        return 8;
    }
}

uint8_t masm_abi_stack_align(MasmTarget target)
{
    switch (target.abi)
    {
    case MASM_ABI_SYSV64:
        return 16;
    default:
        return 16;
    }
}

bool masm_abi_has_red_zone(MasmTarget target)
{
    switch (target.abi)
    {
    case MASM_ABI_SYSV64:
        return true;
    default:
        return false;
    }
}

uint8_t masm_abi_int_arg_count(MasmTarget target)
{
    switch (target.abi)
    {
    case MASM_ABI_SYSV64:
        return 6;
    default:
        return 0;
    }
}

uint32_t masm_abi_int_arg_reg(MasmTarget target, int index)
{
    switch (target.abi)
    {
    case MASM_ABI_SYSV64:
        return masm_sysv64_arg_reg(index);
    default:
        return UINT32_MAX;
    }
}

uint8_t masm_abi_int_ret_count(MasmTarget target)
{
    switch (target.abi)
    {
    case MASM_ABI_SYSV64:
        return 2;
    default:
        return 0;
    }
}

uint32_t masm_abi_int_ret_reg(MasmTarget target, int index)
{
    switch (target.abi)
    {
    case MASM_ABI_SYSV64:
        return masm_sysv64_ret_reg(index);
    default:
        return UINT32_MAX;
    }
}

uint32_t masm_target_stack_pointer_reg(MasmTarget target)
{
    switch (target.isa)
    {
    case MASM_ISA_X86_64:
        return MASM_X86_RSP;
    default:
        return UINT32_MAX;
    }
}

uint32_t masm_target_frame_pointer_reg(MasmTarget target)
{
    switch (target.isa)
    {
    case MASM_ISA_X86_64:
        return MASM_X86_RBP;
    default:
        return UINT32_MAX;
    }
}
