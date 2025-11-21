#ifndef BACKEND_TARGET_H
#define BACKEND_TARGET_H

#include <stdbool.h>

// forward declarations
typedef struct MirModule            MirModule;
typedef struct BackendCodegenResult BackendCodegenResult;
struct Target;

typedef struct TargetISA
{
    bool (*lower)(const struct Target *target, MirModule *module, BackendCodegenResult *result);
} TargetISA;

typedef struct TargetABI
{
    const char *name;
} TargetABI;

typedef struct ObjectWriter
{
    bool (*write_executable)(const struct Target *target, BackendCodegenResult *result, const char *path);
} ObjectWriter;

typedef struct RuntimeShim
{
    const char *entry_label; // e.g. "_start"
} RuntimeShim;

// architecture enum
typedef enum TargetArch
{
    TARGET_ARCH_X86_64,
    TARGET_ARCH_COUNT
} TargetArch;

// operating system / platform enum
typedef enum TargetOS
{
    TARGET_OS_LINUX,
    TARGET_OS_COUNT
} TargetOS;

// binary format enum
typedef enum TargetObjectFormat
{
    TARGET_OBJ_ELF,
    TARGET_OBJ_COUNT
} TargetObjectFormat;

// descriptor for selecting a target
typedef struct TargetDescriptor
{
    TargetArch arch;
    TargetOS   os;
} TargetDescriptor;

// fully described backend target
typedef struct Target
{
    TargetDescriptor   desc;
    TargetObjectFormat format;

    const TargetISA    *isa;
    const TargetABI    *abi;
    const ObjectWriter *writer;
    const RuntimeShim  *runtime;
} Target;

// registry api
const Target *target_lookup(TargetDescriptor desc);
bool          target_register(const Target *target);

// helper for building descriptor
static inline TargetDescriptor target_desc(TargetArch arch, TargetOS os)
{
    TargetDescriptor d = {arch, os};
    return d;
}

#endif // BACKEND_TARGET_H
