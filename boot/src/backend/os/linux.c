#include "backend/os/linux.h"

const TargetOS *backend_os_linux_x86_64(void)
{
    static TargetOS os = {
        .kind           = TARGET_OS_KIND_LINUX,
        .name          = "linux",
        .elf_osabi      = 0x00, // ELFOSABI_SYSV
        .syscall_conv   = "linux-x64",
        .syscall_opcode = {0x0F, 0x05, 0, 0}, // x86-64 syscall instruction
        .syscall_len    = 2
    };
    return &os;
}
