#include "backend/abi/sysv64.h"

const TargetABI *abi_sysv64(void)
{
    static TargetABI abi;
    abi.kind = TARGET_ABI_KIND_SYSV64;
    abi.name = "sysv64";

    return &abi;
}
