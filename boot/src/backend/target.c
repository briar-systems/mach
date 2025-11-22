#include "backend/target.h"
#include "backend/isa/x86_64.h"
#include "backend/os/linux.h"
#include "backend/object/elf64.h"

#include <stddef.h>

// determine the native target for the current system
const Target *target_native()
{
#if defined(__x86_64__) || defined(_M_X64)
    #if defined(__linux__)
        return target_get(TARGET_ARCH_KIND_X86_64, TARGET_OS_KIND_LINUX);
    #else
        return NULL; // unsupported native target
    #endif
#else
    return NULL; // unsupported native architecture
#endif
}

const Target *target_get(TargetArchKind arch, TargetOSKind os)
{
    // compose target from arch + os + format components
    if (arch == TARGET_ARCH_KIND_X86_64 && os == TARGET_OS_KIND_LINUX)
    {
        static RuntimeShim runtime = {.entry_label = "_start"};
        static TargetObjectFormat format = {
            .kind  = TARGET_OBJ_KIND_ELF,
            .name = "elf"
        };
        static Target target;

        target.arch    = backend_arch_x86_64();
        target.os      = backend_os_linux_x86_64();
        target.format  = &format;
        target.isa     = backend_isa_x86_64();
        target.abi     = backend_abi_sysv64();
        target.writer  = backend_object_writer_elf64();
        target.runtime = &runtime;

        return &target;
    }

    // future: add support for other arch/os combinations
    // if (arch == TARGET_ARCH_KIND_X86_64 && os == TARGET_OS_KIND_DARWIN)
    // {
    //     target.arch   = backend_arch_x86_64();
    //     target.os     = backend_os_darwin_x86_64();
    //     target.format = backend_format_macho();
    //     target.isa    = backend_isa_x86_64();
    //     target.abi    = backend_abi_sysv64();
    //     ...
    // }

    return NULL;
}

const Target *target_lookup(TargetDescriptor desc)
{
    return target_get(desc.arch, desc.os);
}
