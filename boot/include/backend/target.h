#ifndef TARGET_H
#define TARGET_H

#include <stdbool.h>
#include <stdint.h>

// target ABI kinds
typedef enum TargetABIKind
{
    TARGET_ABI_KIND_SYSV64,
    TARGET_ABI_KIND_COUNT
} TargetABIKind;

// string representations of target ABIs
static const char *TARGET_ABI_NAMES[] = {
    "sysv64",
};

// represents information about a specific application binary interface
typedef struct TargetABI
{
    TargetABIKind kind;
    const char   *name;
} TargetABI;

// target ISA kinds
typedef enum TargetISAKind
{
    TARGET_ISA_KIND_X86_64,
    TARGET_ISA_KIND_COUNT
} TargetISAKind;

// string representations of target ISAs
static const char *TARGET_ISA_NAMES[] = {
    "x86_64",
};

// represents information about a specific instruction set architecture
typedef struct TargetISA
{
    TargetISAKind kind;
    const char   *name;
    uint8_t       pointer_size;
} TargetISA;

// target object format kinds
typedef enum TargetOFKind
{
    TARGET_OBJ_KIND_ELF,
    TARGET_OBJ_KIND_COUNT
} TargetOFKind;

// string representations of target object formats
static const char *TARGET_OBJ_NAMES[] = {
    "elf",
};

// represents information about a specific object format
typedef struct TargetOF
{
    TargetOFKind kind;
    const char  *name;
} TargetOF;

// target OS kinds
typedef enum TargetOSKind
{
    TARGET_OS_KIND_LINUX,
    TARGET_OS_KIND_COUNT
} TargetOSKind;

// string representations of target operating systems
static const char *TARGET_OS_NAMES[] = {
    "linux",
};

// represents information about a specific operating system
typedef struct TargetOS
{
    TargetOSKind kind;
    const char  *name;

    const TargetOF *format;
} TargetOS;

// represents a specific target configuration
typedef struct Target
{
    const TargetOS  *os;
    const TargetISA *isa;
    const TargetABI *abi;
} Target;

const Target *target_get(TargetISAKind isa, TargetABIKind abi, TargetOSKind os);
const Target *target_native();

const char *target_abi_name(TargetABIKind abi);
const char *target_isa_name(TargetISAKind isa);
const char *target_os_name(TargetOSKind os);
const char *target_of_name(TargetOFKind of);

#endif
