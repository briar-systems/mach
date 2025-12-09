#include "compiler/masm/abi/abi.h"
#include "compiler/masm/isa/x86_64.h"

// sysv64 spec
static const uint32_t SYSV64_INT_ARGS[] = {
    MASM_X86_RDI,
    MASM_X86_RSI,
    MASM_X86_RDX,
    MASM_X86_RCX,
    MASM_X86_R8,
    MASM_X86_R9,
};

static const uint32_t SYSV64_INT_RETS[] = {
    MASM_X86_RAX,
    MASM_X86_RDX,
};

static const MasmABISpec ABI_SYSV64 = {
    .pointer_size  = 8,
    .stack_align   = 16,
    .has_red_zone  = true,
    .int_arg_regs  = SYSV64_INT_ARGS,
    .int_arg_count = sizeof(SYSV64_INT_ARGS) / sizeof(SYSV64_INT_ARGS[0]),
    .int_ret_regs  = SYSV64_INT_RETS,
    .int_ret_count = sizeof(SYSV64_INT_RETS) / sizeof(SYSV64_INT_RETS[0]),
};

const MasmABISpec *masm_abi_spec_select(MasmTarget target)
{
    switch (target.abi)
    {
    case MASM_ABI_SYSV64:
        return &ABI_SYSV64;
    default:
        return NULL;
    }
}

static const MasmABISpec *abi_spec_or_null(MasmTarget target)
{
    return masm_abi_spec_select(target);
}

uint8_t masm_abi_pointer_size(MasmTarget target)
{
    const MasmABISpec *abi = abi_spec_or_null(target);
    return abi ? abi->pointer_size : 0;
}

uint8_t masm_abi_stack_align(MasmTarget target)
{
    const MasmABISpec *abi = abi_spec_or_null(target);
    return abi ? abi->stack_align : 0;
}

bool masm_abi_has_red_zone(MasmTarget target)
{
    const MasmABISpec *abi = abi_spec_or_null(target);
    return abi ? abi->has_red_zone : false;
}

uint8_t masm_abi_int_arg_count(MasmTarget target)
{
    const MasmABISpec *abi = abi_spec_or_null(target);
    return abi ? abi->int_arg_count : 0;
}

uint32_t masm_abi_int_arg_reg(MasmTarget target, int index)
{
    const MasmABISpec *abi = abi_spec_or_null(target);
    if (!abi || index < 0 || index >= abi->int_arg_count) return UINT32_MAX;
    return abi->int_arg_regs[index];
}

uint8_t masm_abi_int_ret_count(MasmTarget target)
{
    const MasmABISpec *abi = abi_spec_or_null(target);
    return abi ? abi->int_ret_count : 0;
}

uint32_t masm_abi_int_ret_reg(MasmTarget target, int index)
{
    const MasmABISpec *abi = abi_spec_or_null(target);
    if (!abi || index < 0 || index >= abi->int_ret_count) return UINT32_MAX;
    return abi->int_ret_regs[index];
}
