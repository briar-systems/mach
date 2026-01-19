#include "compiler/masm/target.h"
#include <string.h>

// string name tables
static const char *MASM_ISA_NAMES[] = {"x86_64"};
static const char *MASM_ABI_NAMES[] = {"sysv64"};
static const char *MASM_OS_NAMES[] = {"linux"};
static const char *MASM_OF_NAMES[] = {"elf"};

MasmTarget masm_target_create(MasmTargetISA isa, MasmTargetABI abi, MasmTargetOS os, MasmTargetOF of)
{
    MasmTarget target;
    target.isa = isa;
    target.abi = abi;
    target.os = os;
    target.of = of;
    return target;
}

MasmTarget masm_target_native()
{
    MasmTargetISA isa = MASM_ISA_COUNT;
    MasmTargetABI abi = MASM_ABI_COUNT;
    MasmTargetOS os = MASM_OS_COUNT;
    MasmTargetOF of = MASM_OF_COUNT;

#if defined(__x86_64__) || defined(_M_X64)
    isa = MASM_ISA_X86_64;
#endif

#if defined(__linux__)
    abi = MASM_ABI_SYSV64;
    os = MASM_OS_LINUX;
    of = MASM_OF_ELF;
#endif

    return masm_target_create(isa, abi, os, of);
}

const char *masm_target_isa_name(MasmTargetISA isa)
{
    if (isa >= MASM_ISA_COUNT)
    {
        return "unknown";
    }
    return MASM_ISA_NAMES[isa];
}

const char *masm_target_abi_name(MasmTargetABI abi)
{
    if (abi >= MASM_ABI_COUNT)
    {
        return "unknown";
    }
    return MASM_ABI_NAMES[abi];
}

const char *masm_target_os_name(MasmTargetOS os)
{
    if (os >= MASM_OS_COUNT)
    {
        return "unknown";
    }
    return MASM_OS_NAMES[os];
}

const char *masm_target_of_name(MasmTargetOF of)
{
    if (of >= MASM_OF_COUNT)
    {
        return "unknown";
    }
    return MASM_OF_NAMES[of];
}

MasmTargetISA masm_target_isa_from_name(const char *name)
{
    if (!name)
    {
        return MASM_ISA_COUNT;
    }

    for (int i = 0; i < MASM_ISA_COUNT; i++)
    {
        if (strcmp(MASM_ISA_NAMES[i], name) == 0)
        {
            return (MasmTargetISA)i;
        }
    }

    return MASM_ISA_COUNT;
}

MasmTargetABI masm_target_abi_from_name(const char *name)
{
    if (!name)
    {
        return MASM_ABI_COUNT;
    }

    for (int i = 0; i < MASM_ABI_COUNT; i++)
    {
        if (strcmp(MASM_ABI_NAMES[i], name) == 0)
        {
            return (MasmTargetABI)i;
        }
    }

    return MASM_ABI_COUNT;
}

MasmTargetOS masm_target_os_from_name(const char *name)
{
    if (!name)
    {
        return MASM_OS_COUNT;
    }

    for (int i = 0; i < MASM_OS_COUNT; i++)
    {
        if (strcmp(MASM_OS_NAMES[i], name) == 0)
        {
            return (MasmTargetOS)i;
        }
    }

    return MASM_OS_COUNT;
}

MasmTargetOF masm_target_of_from_name(const char *name)
{
    if (!name)
    {
        return MASM_OF_COUNT;
    }

    for (int i = 0; i < MASM_OF_COUNT; i++)
    {
        if (strcmp(MASM_OF_NAMES[i], name) == 0)
        {
            return (MasmTargetOF)i;
        }
    }

    return MASM_OF_COUNT;
}
