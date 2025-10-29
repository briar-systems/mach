#include "comptime.h"
#include <string.h>

// Detect host platform at compile time
#if defined(__linux__)
#define HOST_OS COMPTIME_OS_LINUX
#define HOST_OS_STRING "linux"
#elif defined(__APPLE__) && defined(__MACH__)
#define HOST_OS COMPTIME_OS_DARWIN
#define HOST_OS_STRING "darwin"
#elif defined(_WIN32) || defined(_WIN64)
#define HOST_OS COMPTIME_OS_WINDOWS
#define HOST_OS_STRING "windows"
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#define HOST_OS COMPTIME_OS_BSD
#define HOST_OS_STRING "bsd"
#else
#define HOST_OS COMPTIME_OS_UNKNOWN
#define HOST_OS_STRING "unknown"
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define HOST_ARCH COMPTIME_ARCH_X86_64
#define HOST_ARCH_STRING "x86_64"
#define HOST_POINTER_SIZE 8
#elif defined(__aarch64__) || defined(_M_ARM64)
#define HOST_ARCH COMPTIME_ARCH_ARM64
#define HOST_ARCH_STRING "arm64"
#define HOST_POINTER_SIZE 8
#elif defined(__arm__) || defined(_M_ARM)
#define HOST_ARCH COMPTIME_ARCH_ARM
#define HOST_ARCH_STRING "arm"
#define HOST_POINTER_SIZE 4
#elif defined(__riscv) && (__riscv_xlen == 64)
#define HOST_ARCH COMPTIME_ARCH_RISCV64
#define HOST_ARCH_STRING "riscv64"
#define HOST_POINTER_SIZE 8
#else
#define HOST_ARCH COMPTIME_ARCH_UNKNOWN
#define HOST_ARCH_STRING "unknown"
#define HOST_POINTER_SIZE 8
#endif

#define MACH_VERSION "0.1.0"

// Helper to get OS string from enum
static const char *comptime_os_string(ComptimeOS os)
{
    switch (os)
    {
    case COMPTIME_OS_LINUX:
        return "linux";
    case COMPTIME_OS_DARWIN:
        return "darwin";
    case COMPTIME_OS_WINDOWS:
        return "windows";
    case COMPTIME_OS_BSD:
        return "bsd";
    default:
        return "unknown";
    }
}

// Helper to get arch string from enum
static const char *comptime_arch_string(ComptimeArch arch)
{
    switch (arch)
    {
    case COMPTIME_ARCH_X86_64:
        return "x86_64";
    case COMPTIME_ARCH_ARM64:
        return "arm64";
    case COMPTIME_ARCH_ARM:
        return "arm";
    case COMPTIME_ARCH_RISCV64:
        return "riscv64";
    default:
        return "unknown";
    }
}

// Helper to get pointer size from arch
static unsigned int comptime_arch_pointer_size(ComptimeArch arch)
{
    switch (arch)
    {
    case COMPTIME_ARCH_X86_64:
    case COMPTIME_ARCH_ARM64:
    case COMPTIME_ARCH_RISCV64:
        return 8;
    case COMPTIME_ARCH_ARM:
        return 4;
    default:
        return 8;
    }
}

void comptime_build_context_init_host(ComptimeBuildContext *ctx)
{
    if (!ctx)
        return;

    ctx->target_os           = HOST_OS;
    ctx->target_arch         = HOST_ARCH;
    ctx->target_pointer_size = HOST_POINTER_SIZE;
    ctx->target_triple       = HOST_ARCH_STRING "-unknown-" HOST_OS_STRING;
    ctx->build_debug         = true;
    ctx->opt_level           = 0;
    ctx->mach_version        = MACH_VERSION;
}

bool comptime_get_constant(const char *name, ComptimeValue *out_value, const ComptimeBuildContext *ctx)
{
    if (!name || !out_value)
        return false;

    // Default to host context if none provided
    static ComptimeBuildContext default_ctx;
    static bool                 default_initialized = false;
    if (!ctx)
    {
        if (!default_initialized)
        {
            comptime_build_context_init_host(&default_ctx);
            default_initialized = true;
        }
        ctx = &default_ctx;
    }

    // OS enum values (constants, not dependent on target)
    if (strcmp(name, "OS_LINUX") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = COMPTIME_OS_LINUX;
        return true;
    }
    if (strcmp(name, "OS_DARWIN") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = COMPTIME_OS_DARWIN;
        return true;
    }
    if (strcmp(name, "OS_WINDOWS") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = COMPTIME_OS_WINDOWS;
        return true;
    }
    if (strcmp(name, "OS_BSD") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = COMPTIME_OS_BSD;
        return true;
    }

    // ARCH enum values (constants, not dependent on target)
    if (strcmp(name, "ARCH_X86_64") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = COMPTIME_ARCH_X86_64;
        return true;
    }
    if (strcmp(name, "ARCH_ARM64") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = COMPTIME_ARCH_ARM64;
        return true;
    }
    if (strcmp(name, "ARCH_ARM") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = COMPTIME_ARCH_ARM;
        return true;
    }
    if (strcmp(name, "ARCH_RISCV64") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = COMPTIME_ARCH_RISCV64;
        return true;
    }

    // Target information (uses build context)
    if (strcmp(name, "target.os") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = (unsigned char)ctx->target_os;
        return true;
    }
    if (strcmp(name, "target.arch") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = (unsigned char)ctx->target_arch;
        return true;
    }
    if (strcmp(name, "target.pointer_size") == 0)
    {
        out_value->kind    = COMPTIME_U64;
        out_value->u64_val = ctx->target_pointer_size;
        return true;
    }
    if (strcmp(name, "target.word_size") == 0)
    {
        out_value->kind    = COMPTIME_U64;
        out_value->u64_val = ctx->target_pointer_size;
        return true;
    }
    if (strcmp(name, "target.triple") == 0)
    {
        out_value->kind       = COMPTIME_STRING;
        out_value->string_val = ctx->target_triple;
        return true;
    }

    // Build configuration (uses build context)
    if (strcmp(name, "build.debug") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = ctx->build_debug ? 1 : 0;
        return true;
    }

    // Mach language information
    if (strcmp(name, "mach.version") == 0)
    {
        out_value->kind       = COMPTIME_STRING;
        out_value->string_val = ctx->mach_version;
        return true;
    }

    // Legacy constants (for backward compatibility - use target from context)
    if (strcmp(name, "OS") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = (unsigned char)ctx->target_os;
        return true;
    }
    if (strcmp(name, "ARCH") == 0)
    {
        out_value->kind   = COMPTIME_U8;
        out_value->u8_val = (unsigned char)ctx->target_arch;
        return true;
    }
    if (strcmp(name, "PTR_WIDTH") == 0)
    {
        out_value->kind    = COMPTIME_U64;
        out_value->u64_val = ctx->target_pointer_size * 8;
        return true;
    }

    return false;
}
