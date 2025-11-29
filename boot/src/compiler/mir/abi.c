#include "compiler/mir/abi.h"
#include "compiler/mir/abi/sysv64.h"

int mir_abi_classify_param(MIRTargetABI abi, Type *type)
{
    switch (abi)
    {
    case MIR_ABI_SYSV64:
        return sysv64_classify_param(type);
    default:
        return 1; // fallback: single register
    }
}

int mir_abi_classify_return(MIRTargetABI abi, Type *type)
{
    switch (abi)
    {
    case MIR_ABI_SYSV64:
        return sysv64_classify_return(type);
    default:
        return type ? 1 : 0; // fallback
    }
}
