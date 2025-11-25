#include "compiler/mir/target.h"
#include <string.h>

// string name tables
static const char *MIR_ISA_NAMES[] = {"x86_64"};
static const char *MIR_ABI_NAMES[] = {"sysv64"};
static const char *MIR_OS_NAMES[] = {"linux"};
static const char *MIR_OF_NAMES[] = {"elf"};

MIRTarget mir_target_create(MIRTargetISA isa, MIRTargetABI abi, MIRTargetOS os, MIRTargetOF of)
{
    MIRTarget target;
    target.isa = isa;
    target.abi = abi;
    target.os = os;
    target.of = of;
    return target;
}

MIRTarget mir_target_native()
{
    MIRTargetISA isa = MIR_ISA_COUNT;
    MIRTargetABI abi = MIR_ABI_COUNT;
    MIRTargetOS os = MIR_OS_COUNT;
    MIRTargetOF of = MIR_OF_COUNT;

#if defined(__x86_64__) || defined(_M_X64)
    isa = MIR_ISA_X86_64;
#endif

#if defined(__linux__)
    abi = MIR_ABI_SYSV64;
    os = MIR_OS_LINUX;
    of = MIR_OF_ELF;
#endif

    return mir_target_create(isa, abi, os, of);
}

const char *mir_target_isa_name(MIRTargetISA isa)
{
    if (isa >= MIR_ISA_COUNT)
    {
        return "unknown";
    }
    return MIR_ISA_NAMES[isa];
}

const char *mir_target_abi_name(MIRTargetABI abi)
{
    if (abi >= MIR_ABI_COUNT)
    {
        return "unknown";
    }
    return MIR_ABI_NAMES[abi];
}

const char *mir_target_os_name(MIRTargetOS os)
{
    if (os >= MIR_OS_COUNT)
    {
        return "unknown";
    }
    return MIR_OS_NAMES[os];
}

const char *mir_target_of_name(MIRTargetOF of)
{
    if (of >= MIR_OF_COUNT)
    {
        return "unknown";
    }
    return MIR_OF_NAMES[of];
}

MIRTargetISA mir_target_isa_from_name(const char *name)
{
    if (!name)
    {
        return MIR_ISA_COUNT;
    }

    for (int i = 0; i < MIR_ISA_COUNT; i++)
    {
        if (strcmp(MIR_ISA_NAMES[i], name) == 0)
        {
            return (MIRTargetISA)i;
        }
    }

    return MIR_ISA_COUNT;
}

MIRTargetABI mir_target_abi_from_name(const char *name)
{
    if (!name)
    {
        return MIR_ABI_COUNT;
    }

    for (int i = 0; i < MIR_ABI_COUNT; i++)
    {
        if (strcmp(MIR_ABI_NAMES[i], name) == 0)
        {
            return (MIRTargetABI)i;
        }
    }

    return MIR_ABI_COUNT;
}

MIRTargetOS mir_target_os_from_name(const char *name)
{
    if (!name)
    {
        return MIR_OS_COUNT;
    }

    for (int i = 0; i < MIR_OS_COUNT; i++)
    {
        if (strcmp(MIR_OS_NAMES[i], name) == 0)
        {
            return (MIRTargetOS)i;
        }
    }

    return MIR_OS_COUNT;
}

MIRTargetOF mir_target_of_from_name(const char *name)
{
    if (!name)
    {
        return MIR_OF_COUNT;
    }

    for (int i = 0; i < MIR_OF_COUNT; i++)
    {
        if (strcmp(MIR_OF_NAMES[i], name) == 0)
        {
            return (MIRTargetOF)i;
        }
    }

    return MIR_OF_COUNT;
}
