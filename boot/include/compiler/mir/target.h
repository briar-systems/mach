#ifndef MIR_TARGET_H
#define MIR_TARGET_H

#include <stdbool.h>
#include <stdint.h>

// mir compilation target configuration
// defines isa, abi, os, and object file format for code generation

// instruction set architecture kinds
typedef enum MIRTargetISA
{
    MIR_ISA_X86_64,
    MIR_ISA_COUNT
} MIRTargetISA;

// application binary interface kinds
typedef enum MIRTargetABI
{
    MIR_ABI_SYSV64,
    MIR_ABI_COUNT
} MIRTargetABI;

// operating system kinds
typedef enum MIRTargetOS
{
    MIR_OS_LINUX,
    MIR_OS_COUNT
} MIRTargetOS;

// object file format kinds
typedef enum MIRTargetOF
{
    MIR_OF_ELF,
    MIR_OF_COUNT
} MIRTargetOF;

// complete target specification
typedef struct MIRTarget
{
    MIRTargetISA isa;
    MIRTargetABI abi;
    MIRTargetOS  os;
    MIRTargetOF  of;
} MIRTarget;

// target construction
MIRTarget mir_target_create(MIRTargetISA isa, MIRTargetABI abi, MIRTargetOS os, MIRTargetOF of);
MIRTarget mir_target_native();

// string conversions
const char   *mir_target_isa_name(MIRTargetISA isa);
const char   *mir_target_abi_name(MIRTargetABI abi);
const char   *mir_target_os_name(MIRTargetOS os);
const char   *mir_target_of_name(MIRTargetOF of);
MIRTargetISA  mir_target_isa_from_name(const char *name);
MIRTargetABI  mir_target_abi_from_name(const char *name);
MIRTargetOS   mir_target_os_from_name(const char *name);
MIRTargetOF   mir_target_of_from_name(const char *name);

#endif // MIR_TARGET_H
