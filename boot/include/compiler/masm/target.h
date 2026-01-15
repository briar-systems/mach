#ifndef MASM_TARGET_H
#define MASM_TARGET_H

#include <stdbool.h>
#include <stdint.h>

// masm compilation target configuration
// defines isa, abi, os, and object file format for code generation

// instruction set architecture kinds
typedef enum MasmTargetISA
{
    MASM_ISA_X86_64,
    MASM_ISA_COUNT
} MasmTargetISA;

// application binary interface kinds
typedef enum MasmTargetABI
{
    MASM_ABI_SYSV64,
    MASM_ABI_COUNT
} MasmTargetABI;

// operating system kinds
typedef enum MasmTargetOS
{
    MASM_OS_LINUX,
    MASM_OS_COUNT
} MasmTargetOS;

// object file format kinds
typedef enum MasmTargetOF
{
    MASM_OF_ELF,
    MASM_OF_COUNT
} MasmTargetOF;

// complete target specification
typedef struct MasmTarget
{
    MasmTargetISA isa;
    MasmTargetABI abi;
    MasmTargetOS  os;
    MasmTargetOF  of;
} MasmTarget;

// target construction
MasmTarget masm_target_create(MasmTargetISA isa, MasmTargetABI abi, MasmTargetOS os, MasmTargetOF of);
MasmTarget masm_target_native();

// string conversions
const char   *masm_target_isa_name(MasmTargetISA isa);
const char   *masm_target_abi_name(MasmTargetABI abi);
const char   *masm_target_os_name(MasmTargetOS os);
const char   *masm_target_of_name(MasmTargetOF of);
MasmTargetISA masm_target_isa_from_name(const char *name);
MasmTargetABI masm_target_abi_from_name(const char *name);
MasmTargetOS  masm_target_os_from_name(const char *name);
MasmTargetOF  masm_target_of_from_name(const char *name);

#endif // MASM_TARGET_H
