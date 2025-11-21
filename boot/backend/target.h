#ifndef BACKEND_TARGET_H
#define BACKEND_TARGET_H

#include <stdbool.h>

// forward declarations
typedef struct MirModule MirModule;
typedef struct BackendCodegenResult BackendCodegenResult;
struct BackendTarget;

typedef struct TargetISA
{
    bool (*lower)(const struct BackendTarget *target, MirModule *module, BackendCodegenResult *result);
} TargetISA;

typedef struct TargetABI
{
    const char *name;
} TargetABI;

typedef struct ObjectWriter
{
    bool (*write_executable)(const struct BackendTarget *target,
                             BackendCodegenResult          *result,
                             const char                    *path);
} ObjectWriter;

typedef struct RuntimeShim
{
    const char *entry_label; // e.g. "_start"
} RuntimeShim;

// architecture enum
typedef enum TargetArch
{
    TARGET_ARCH_X86_64,
    TARGET_ARCH_AARCH64,
    TARGET_ARCH_RISCV64,
    TARGET_ARCH_COUNT
} TargetArch;

// operating system / platform enum
typedef enum TargetOS
{
    TARGET_OS_LINUX,
    TARGET_OS_MACOS,
    TARGET_OS_WINDOWS,
    TARGET_OS_COUNT
} TargetOS;

// binary format enum
typedef enum TargetObjectFormat
{
    TARGET_OBJ_ELF,
    TARGET_OBJ_MACHO,
    TARGET_OBJ_PE,
    TARGET_OBJ_COUNT
} TargetObjectFormat;

// descriptor for selecting a target
typedef struct TargetDescriptor
{
    TargetArch arch;
    TargetOS   os;
} TargetDescriptor;

// fully described backend target
typedef struct BackendTarget
{
    TargetDescriptor    desc;
    TargetObjectFormat  format;

    const TargetISA    *isa;
    const TargetABI    *abi;
    const ObjectWriter *writer;
    const RuntimeShim  *runtime;
} BackendTarget;

// registry api
const BackendTarget *backend_target_lookup(TargetDescriptor desc);
bool backend_target_register(const BackendTarget *target);

// helper for building descriptor
static inline TargetDescriptor target_desc(TargetArch arch, TargetOS os)
{
    TargetDescriptor d = {arch, os};
    return d;
}

#endif // BACKEND_TARGET_H
