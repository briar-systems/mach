#ifndef TARGET_H
#define TARGET_H

#include <stdbool.h>
#include <stdint.h>

// forward declarations
typedef struct MirModule            MirModule;
typedef struct BackendCodegenResult BackendCodegenResult;
struct Target;

typedef enum TargetABIKind
{
    TARGET_ABI_KIND_SYSV64,
    TARGET_ABI_KIND_COUNT
} TargetABIKind;

typedef struct TargetABI
{
    TargetABIKind kind;
    const char   *name;
} TargetABI;

typedef enum TargetISAKind
{
    TARGET_ISA_KIND_X86_64,
    TARGET_ISA_KIND_COUNT
} TargetISAKind;

typedef struct TargetISA
{
    TargetISAKind kind;
    const char   *name;         // string representation ("x86_64", "arm64", etc.)
    uint16_t      elf_machine;  // ELF e_machine value (EM_X86_64, EM_AARCH64, etc.)
    uint8_t       pointer_size; // pointer size in bytes

    bool (*lower)(const struct Target *target, MirModule *module, BackendCodegenResult *result);
} TargetISA;

// operating system / platform enum
typedef enum TargetOSKind
{
    TARGET_OS_KIND_LINUX,
    TARGET_OS_KIND_COUNT
} TargetOSKind;

// os-specific information
typedef struct TargetOS
{
    TargetOSKind kind;
    const char  *name;              // string representation ("linux", "darwin", "windows", etc.)
    uint8_t      elf_osabi;         // ELF EI_OSABI value (ELFOSABI_SYSV, ELFOSABI_FREEBSD, etc.)
    const char  *syscall_conv;      // syscall convention identifier ("linux-x64", "darwin-x64", etc.)
    uint8_t      syscall_opcode[4]; // syscall instruction bytes (e.g., {0x0F, 0x05} for linux x64)
    uint8_t      syscall_len;       // length of syscall instruction in bytes
} TargetOS;

typedef struct ObjectWriter
{
    bool (*write_executable)(const struct Target *target, BackendCodegenResult *result, const char *path);
} ObjectWriter;

typedef struct RuntimeShim
{
    const char *entry_label;
} RuntimeShim;

// binary format enum
typedef enum TargetObjectFormatKind
{
    TARGET_OBJ_KIND_ELF,
    TARGET_OBJ_KIND_COUNT
} TargetObjectFormatKind;

// object format information
typedef struct TargetObjectFormat
{
    TargetObjectFormatKind kind;
    const char            *name; // string representation ("elf", "macho", "pe", etc.)
} TargetObjectFormat;

// fully described backend target
typedef struct Target
{
    const TargetISA          *isa;
    const TargetABI          *abi;
    const TargetOS           *os;
    const TargetObjectFormat *format;

    const ObjectWriter *writer;
    const RuntimeShim  *runtime;
} Target;

typedef struct TargetDescriptor
{
    TargetISAKind isa;
    TargetABIKind abi;
    TargetOSKind  os;
} TargetDescriptor;

const Target *target_get(TargetDescriptor desc);

TargetDescriptor target_native_descriptor();
TargetDescriptor target_get_descriptor(const Target *target);

#endif
