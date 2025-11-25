#ifndef MIR_ISA_X86_64_H
#define MIR_ISA_X86_64_H

#include <stdint.h>
#include <stdbool.h>

// x86_64 physical register ids
typedef enum X86_64_Reg
{
    // general purpose (64-bit)
    X86_64_RAX,
    X86_64_RBX,
    X86_64_RCX,
    X86_64_RDX,
    X86_64_RSI,
    X86_64_RDI,
    X86_64_RBP,
    X86_64_RSP,
    X86_64_R8,
    X86_64_R9,
    X86_64_R10,
    X86_64_R11,
    X86_64_R12,
    X86_64_R13,
    X86_64_R14,
    X86_64_R15,

    // floating point (xmm)
    X86_64_XMM0,
    X86_64_XMM1,
    X86_64_XMM2,
    X86_64_XMM3,
    X86_64_XMM4,
    X86_64_XMM5,
    X86_64_XMM6,
    X86_64_XMM7,
    X86_64_XMM8,
    X86_64_XMM9,
    X86_64_XMM10,
    X86_64_XMM11,
    X86_64_XMM12,
    X86_64_XMM13,
    X86_64_XMM14,
    X86_64_XMM15,

    X86_64_REG_COUNT
} X86_64_Reg;

// register utilities
const char *x86_64_reg_name(X86_64_Reg reg);
X86_64_Reg  x86_64_reg_from_name(const char *name);
bool        x86_64_reg_is_gp(X86_64_Reg reg);
bool        x86_64_reg_is_fp(X86_64_Reg reg);

// register info for allocator
int x86_64_get_gp_reg_count();
int x86_64_get_fp_reg_count();
int x86_64_get_pointer_size();

#endif // MIR_ISA_X86_64_H
