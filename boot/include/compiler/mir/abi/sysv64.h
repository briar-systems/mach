#ifndef MIR_ABI_SYSV64_H
#define MIR_ABI_SYSV64_H

#include <stdbool.h>
#include <stddef.h>
#include "compiler/type.h"

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

// parameter passing classification
// returns number of 8-byte chunks a type occupies in registers (0 = passed by reference)
int sysv64_classify_param(Type *type);

// returns number of 8-byte chunks a type occupies for return (0 = return via hidden pointer)
int sysv64_classify_return(Type *type);

#endif // MIR_ABI_SYSV64_H
