#include "compiler/mir/os/linux.h"
#include <string.h>

// linux x86_64 syscall numbers (subset)
typedef struct
{
    const char *name;
    int         number;
} LinuxSyscall;

static const LinuxSyscall LINUX_SYSCALLS[] = {
    {"read", 0},
    {"write", 1},
    {"open", 2},
    {"close", 3},
    {"mmap", 9},
    {"munmap", 11},
    {"exit", 60},
};

int linux_get_syscall_number(const char *syscall_name)
{
    if (!syscall_name)
    {
        return -1;
    }

    for (size_t i = 0; i < sizeof(LINUX_SYSCALLS) / sizeof(LINUX_SYSCALLS[0]); i++)
    {
        if (strcmp(LINUX_SYSCALLS[i].name, syscall_name) == 0)
        {
            return LINUX_SYSCALLS[i].number;
        }
    }

    return -1;
}

int linux_get_syscall_arg_reg_count()
{
    return 6; // linux uses up to 6 registers for syscall arguments
}

const char *linux_get_entry_point_name()
{
    return "_start"; // linux entry point is _start
}

bool linux_requires_plt()
{
    return false; // for now, no plt/got needed for simple executables
}

bool linux_requires_got()
{
    return false;
}
