#ifndef MIR_OS_LINUX_H
#define MIR_OS_LINUX_H

#include <stdbool.h>

// linux operating system interface

// system calls
int linux_get_syscall_number(const char *syscall_name);
int linux_get_syscall_arg_reg_count();

// program startup
const char *linux_get_entry_point_name();

// linking requirements
bool linux_requires_plt();
bool linux_requires_got();

#endif // MIR_OS_LINUX_H
