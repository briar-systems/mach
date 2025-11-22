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

// represents information about a specific instruction set architecture
typedef struct TargetISA
{
    TargetISAKind kind;
    const char   *name;
    uint8_t       pointer_size;
} TargetISA;

// target object format kinds
typedef enum TargetObjectFormatKind
{
    TARGET_OBJ_KIND_ELF,
    TARGET_OBJ_KIND_COUNT
} TargetObjectFormatKind;

// represents information about a specific object format
typedef struct TargetObjectFormat
{
    TargetObjectFormatKind kind;
    const char            *name;
} TargetObjectFormat;

// target OS kinds
typedef enum TargetOSKind
{
    TARGET_OS_KIND_LINUX,
    TARGET_OS_KIND_COUNT
} TargetOSKind;

// represents information about a specific operating system
typedef struct TargetOS
{
    TargetOSKind kind;
    const char  *name;

    const TargetObjectFormat *format;
} TargetOS;

// represents a specific target configuration
typedef struct Target
{
    const TargetOS  *os;
    const TargetISA *isa;
    const TargetABI *abi;
} Target;

const Target    *target_get(TargetISAKind isa, TargetABIKind abi, TargetOSKind os);
const Target    *target_native();

#endif
