#ifndef COMPTIME_H
#define COMPTIME_H

#include <stdbool.h>

typedef enum ComptimeOS
{
    COMPTIME_OS_UNKNOWN = 0,
    COMPTIME_OS_LINUX   = 1,
    COMPTIME_OS_DARWIN  = 2,
    COMPTIME_OS_WINDOWS = 3,
    COMPTIME_OS_BSD     = 4,
} ComptimeOS;

typedef enum ComptimeArch
{
    COMPTIME_ARCH_UNKNOWN = 0,
    COMPTIME_ARCH_X86_64  = 1,
    COMPTIME_ARCH_ARM64   = 2,
    COMPTIME_ARCH_ARM     = 3,
    COMPTIME_ARCH_RISCV64 = 4,
} ComptimeArch;

typedef struct ComptimeBuildContext
{
    ComptimeOS   target_os;
    ComptimeArch target_arch;
    unsigned int target_pointer_size;
    const char  *target_triple;

    bool build_debug;
    int  opt_level;

    const char *mach_version;
    const char *build_version;
} ComptimeBuildContext;

typedef enum ComptimeValueKind
{
    COMPTIME_U8,
    COMPTIME_U64,
    COMPTIME_STRING,
} ComptimeValueKind;

typedef struct ComptimeValue
{
    ComptimeValueKind kind;
    union
    {
        unsigned char u8_val;
        unsigned long u64_val;
        const char   *string_val;
    };
} ComptimeValue;

void comptime_build_context_init_host(ComptimeBuildContext *ctx);
void comptime_build_context_init_from_triple(ComptimeBuildContext *ctx, const char *target_triple);
bool comptime_get_constant(const char *name, ComptimeValue *out_value, const ComptimeBuildContext *ctx);

#endif // COMPTIME_H
