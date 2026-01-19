#ifndef MASM_OS_LINUX_H
#define MASM_OS_LINUX_H

// linux syscall numbers (x86_64)
typedef enum MasmLinuxSyscall
{
    MASM_LINUX_SYS_READ = 0,
    MASM_LINUX_SYS_WRITE = 1,
    MASM_LINUX_SYS_OPEN = 2,
    MASM_LINUX_SYS_CLOSE = 3,
    MASM_LINUX_SYS_EXIT = 60
} MasmLinuxSyscall;

#endif // MASM_OS_LINUX_H
