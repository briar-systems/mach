#ifndef MIR_ABI_SYSV64_H
#define MIR_ABI_SYSV64_H

#include <stdbool.h>

// system v amd64 abi (linux, bsd, macos on x86_64)

// stack layout
int sysv64_get_stack_alignment();
int sysv64_get_shadow_space_size();

// calling convention - argument registers
// fills regs array with physical register ids, returns count
int sysv64_get_int_arg_regs(int *regs, int max_count);
int sysv64_get_fp_arg_regs(int *regs, int max_count);

// calling convention - return registers
int sysv64_get_int_ret_regs(int *regs, int max_count);
int sysv64_get_fp_ret_regs(int *regs, int max_count);

// register preservation
bool sysv64_is_caller_saved(int physical_reg_id);
bool sysv64_is_callee_saved(int physical_reg_id);

#endif // MIR_ABI_SYSV64_H
