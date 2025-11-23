#include "mir/target.h"
// #include "backend/isa/x86_64.h"
// #include "backend/abi/sysv64.h"
// #include "backend/os/linux.h"

#include <stddef.h>

const Target *target_get(TargetISAKind isa, TargetABIKind abi, TargetOSKind os)
{
    // check that all components are valid
    if (isa == TARGET_ISA_KIND_COUNT || abi == TARGET_ABI_KIND_COUNT || os == TARGET_OS_KIND_COUNT)
    {
        return NULL;
    }

    static Target target;

    switch (isa)
    {
    case TARGET_ISA_KIND_X86_64:
        // target.isa = isa_x86_64();
        break;
    default:
        return NULL;
    }

    switch (abi)
    {
    case TARGET_ABI_KIND_SYSV64:
        // target.abi = abi_sysv64();
        break;
    default:
        return NULL;
    }

    switch (os)
    {
    case TARGET_OS_KIND_LINUX:
        // target.os = os_linux();
        break;
    default:
        return NULL;
    }

    return &target;
}

const Target *target_native()
{
    static TargetISAKind isa = TARGET_ISA_KIND_COUNT;
    static TargetABIKind abi = TARGET_ABI_KIND_COUNT;
    static TargetOSKind  os  = TARGET_OS_KIND_COUNT;

#if defined(__x86_64__) || defined(_M_X64)
    isa = TARGET_ISA_KIND_X86_64;
#endif
#if defined(__linux__)
    abi = TARGET_ABI_KIND_SYSV64;
    os  = TARGET_OS_KIND_LINUX;
#endif

    return target_get(isa, abi, os);
}

const char *target_abi_name(TargetABIKind abi) {
    if (abi >= TARGET_ABI_KIND_COUNT) {
        return "unknown";
    }

    return TARGET_ABI_NAMES[abi];
}

const char *target_isa_name(TargetISAKind isa) {
    if (isa >= TARGET_ISA_KIND_COUNT) {
        return "unknown";
    }

    return TARGET_ISA_NAMES[isa];
}

const char *target_os_name(TargetOSKind os) {
    if (os >= TARGET_OS_KIND_COUNT) {
        return "unknown";
    }

    return TARGET_OS_NAMES[os];
}

const char *target_of_name(TargetOFKind of) {
    if (of >= TARGET_OBJ_KIND_COUNT) {
        return "unknown";
    }

    return TARGET_OBJ_NAMES[of];
}
