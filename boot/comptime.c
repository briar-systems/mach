#include "comptime.h"
#include <string.h>
#include <stdio.h>

// Detect host platform at compile time
#if defined(__linux__)
#define HOST_OS COMPTIME_OS_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define HOST_OS COMPTIME_OS_DARWIN
#elif defined(_WIN32) || defined(_WIN64)
#define HOST_OS COMPTIME_OS_WINDOWS
#else
#define HOST_OS COMPTIME_OS_UNKNOWN
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define HOST_ARCH COMPTIME_ARCH_X86_64
#define HOST_POINTER_SIZE 8
#elif defined(__aarch64__) || defined(_M_ARM64)
#define HOST_ARCH COMPTIME_ARCH_AARCH64
#define HOST_POINTER_SIZE 8
#elif defined(__arm__) || defined(_M_ARM)
#define HOST_ARCH COMPTIME_ARCH_ARM
#define HOST_POINTER_SIZE 4
#elif defined(__riscv) && (__riscv_xlen == 64)
#define HOST_ARCH COMPTIME_ARCH_RISCV64
#define HOST_POINTER_SIZE 8
#elif defined(__riscv) && (__riscv_xlen == 32)
#define HOST_ARCH COMPTIME_ARCH_RISCV32
#define HOST_POINTER_SIZE 4
#else
#define HOST_ARCH COMPTIME_ARCH_UNKNOWN
#define HOST_POINTER_SIZE 8
#endif

#define MACH_VERSION "0.1.0"

// Create architecture descriptor from enum
ComptimeArchitectureInfo comptime_arch_info(ComptimeArch arch)
{
    ComptimeArchitectureInfo info = {0};
    info.id = arch;
    
    switch (arch)
    {
    case COMPTIME_ARCH_X86_64:
        info.name = "x86_64";
        info.word_size = 8;
        info.is_64bit = true;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_AARCH64:
        info.name = "aarch64";
        info.word_size = 8;
        info.is_64bit = true;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_ARM:
        info.name = "arm";
        info.word_size = 4;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_X86:
        info.name = "x86";
        info.word_size = 4;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_RISCV64:
        info.name = "riscv64";
        info.word_size = 8;
        info.is_64bit = true;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_RISCV32:
        info.name = "riscv32";
        info.word_size = 4;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_WASM64:
        info.name = "wasm64";
        info.word_size = 8;
        info.is_64bit = true;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_WASM32:
        info.name = "wasm32";
        info.word_size = 4;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_THUMB:
    case COMPTIME_ARCH_THUMBV6M:
    case COMPTIME_ARCH_THUMBV7M:
        info.name = arch == COMPTIME_ARCH_THUMB ? "thumb" : 
                    arch == COMPTIME_ARCH_THUMBV6M ? "thumbv6m" : "thumbv7m";
        info.word_size = 4;
        info.is_embedded = true;
        break;
    case COMPTIME_ARCH_THUMBV7EM:
        info.name = "thumbv7em";
        info.word_size = 4;
        info.is_embedded = true;
        info.has_fpu = true;
        break;
    case COMPTIME_ARCH_AVR:
        info.name = "avr";
        info.word_size = 2;
        info.is_embedded = true;
        break;
    case COMPTIME_ARCH_MSP430:
        info.name = "msp430";
        info.word_size = 2;
        info.is_embedded = true;
        break;
    default:
        info.name = "unknown";
        break;
    }
    
    return info;
}

// Create OS descriptor from enum
ComptimeOSInfo comptime_os_info(ComptimeOS os)
{
    ComptimeOSInfo info = {0};
    info.id = os;
    
    switch (os)
    {
    case COMPTIME_OS_LINUX:
        info.name = "linux";
        info.supports_filesystem = true;
        info.supports_networking = true;
        info.supports_threads = true;
        info.has_mmu = true;
        break;
    case COMPTIME_OS_DARWIN:
        info.name = "darwin";
        info.supports_filesystem = true;
        info.supports_networking = true;
        info.supports_threads = true;
        info.has_mmu = true;
        break;
    case COMPTIME_OS_WINDOWS:
        info.name = "windows";
        info.supports_filesystem = true;
        info.supports_networking = true;
        info.supports_threads = true;
        info.has_mmu = true;
        break;
    case COMPTIME_OS_WASI:
        info.name = "wasi";
        info.supports_filesystem = true;
        break;
    case COMPTIME_OS_ZEPHYR:
        info.name = "zephyr";
        info.supports_filesystem = true;
        info.supports_threads = true;
        break;
    case COMPTIME_OS_FREESTANDING:
        info.name = "freestanding";
        info.is_freestanding = true;
        break;
    case COMPTIME_OS_NONE:
        info.name = "none";
        info.is_freestanding = true;
        break;
    default:
        info.name = "unknown";
        break;
    }
    
    return info;
}

// Create ABI descriptor from enum
ComptimeABIInfo comptime_abi_info(ComptimeABI abi)
{
    ComptimeABIInfo info = {0};
    info.id = abi;
    
    switch (abi)
    {
    case COMPTIME_ABI_GNU:
        info.name = "gnu";
        info.requires_libc = true;
        break;
    case COMPTIME_ABI_MSVC:
        info.name = "msvc";
        info.requires_libc = true;
        break;
    case COMPTIME_ABI_MUSL:
        info.name = "musl";
        info.requires_libc = true;
        break;
    case COMPTIME_ABI_NEWLIB:
        info.name = "newlib";
        info.requires_libc = true;
        break;
    case COMPTIME_ABI_BARE:
        info.name = "bare";
        break;
    case COMPTIME_ABI_WASI:
        info.name = "wasi";
        break;
    default:
        info.name = "unknown";
        break;
    }
    
    return info;
}

ComptimeVendorInfo comptime_vendor_info(ComptimeVendor vendor)
{
    ComptimeVendorInfo info = {0};
    info.id = vendor;
    
    switch (vendor)
    {
    case COMPTIME_VENDOR_PC:        info.name = "pc"; break;
    case COMPTIME_VENDOR_APPLE:     info.name = "apple"; break;
    case COMPTIME_VENDOR_MSP:       info.name = "msp"; break;
    case COMPTIME_VENDOR_ESPRESSIF: info.name = "espressif"; break;
    case COMPTIME_VENDOR_CUSTOM:    info.name = "custom"; break;
    default:                        info.name = "unknown"; break;
    }
    
    return info;
}

ComptimeEnvironmentInfo comptime_environment_info(ComptimeEnvironment env)
{
    ComptimeEnvironmentInfo info = {0};
    info.id = env;
    
    switch (env)
    {
    case COMPTIME_ENVIRONMENT_NONE:   info.name = "none"; break;
    case COMPTIME_ENVIRONMENT_GNU:    info.name = "gnu"; break;
    case COMPTIME_ENVIRONMENT_MUSL:   info.name = "musl"; break;
    case COMPTIME_ENVIRONMENT_NEWLIB: info.name = "newlib"; break;
    case COMPTIME_ENVIRONMENT_WASI:   info.name = "wasi"; break;
    case COMPTIME_ENVIRONMENT_BARE:   info.name = "bare"; break;
    default:                          info.name = "unknown"; break;
    }
    
    return info;
}

ComptimeEndiannessInfo comptime_endianness_info(ComptimeEndianness endian)
{
    ComptimeEndiannessInfo info = {0};
    info.id = endian;
    
    switch (endian)
    {
    case COMPTIME_ENDIANNESS_LITTLE: info.name = "little"; break;
    case COMPTIME_ENDIANNESS_BIG:    info.name = "big"; break;
    case COMPTIME_ENDIANNESS_MIXED:  info.name = "mixed"; break;
    default:                         info.name = "unknown"; break;
    }
    
    return info;
}

ComptimeObjectFormatInfo comptime_object_format_info(ComptimeObjectFormat fmt)
{
    ComptimeObjectFormatInfo info = {0};
    info.id = fmt;
    
    switch (fmt)
    {
    case COMPTIME_OBJECT_FORMAT_ELF:    info.name = "elf"; break;
    case COMPTIME_OBJECT_FORMAT_PE:     info.name = "pe"; break;
    case COMPTIME_OBJECT_FORMAT_MACHO:  info.name = "macho"; break;
    case COMPTIME_OBJECT_FORMAT_WASM:   info.name = "wasm"; break;
    case COMPTIME_OBJECT_FORMAT_RAWBIN: info.name = "rawbin"; break;
    case COMPTIME_OBJECT_FORMAT_HEX:    info.name = "hex"; break;
    case COMPTIME_OBJECT_FORMAT_UF2:    info.name = "uf2"; break;
    default:                            info.name = "unknown"; break;
    }
    
    return info;
}

ComptimeBackendKindInfo comptime_backend_kind_info(ComptimeBackendKind kind)
{
    ComptimeBackendKindInfo info = {0};
    info.id = kind;
    
    switch (kind)
    {
    case COMPTIME_BACKEND_KIND_NATIVE:   info.name = "native"; break;
    case COMPTIME_BACKEND_KIND_WASM:     info.name = "wasm"; break;
    case COMPTIME_BACKEND_KIND_EMBEDDED: info.name = "embedded"; break;
    case COMPTIME_BACKEND_KIND_CUSTOM:   info.name = "custom"; break;
    default:                             info.name = "unknown"; break;
    }
    
    return info;
}

ComptimeFeatureInfo comptime_feature_info(ComptimeFeature feat)
{
    ComptimeFeatureInfo info = {0};
    info.id = feat;
    
    switch (feat)
    {
    case COMPTIME_FEATURE_SOFT_FLOAT:       info.name = "soft_float"; break;
    case COMPTIME_FEATURE_HARD_FLOAT:       info.name = "hard_float"; break;
    case COMPTIME_FEATURE_SIMD:             info.name = "simd"; break;
    case COMPTIME_FEATURE_UNALIGNED_MEMORY: info.name = "unaligned_memory"; break;
    case COMPTIME_FEATURE_ATOMICS_64:       info.name = "atomics_64"; break;
    case COMPTIME_FEATURE_ATOMICS_128:      info.name = "atomics_128"; break;
    case COMPTIME_FEATURE_THREADS:          info.name = "threads"; break;
    case COMPTIME_FEATURE_MMU:              info.name = "mmu"; break;
    case COMPTIME_FEATURE_CACHE:            info.name = "cache"; break;
    case COMPTIME_FEATURE_VECTOR_EXT:       info.name = "vector_ext"; break;
    default:                                info.name = "unknown"; break;
    }
    
    return info;
}

// Global namespace instances
const ComptimeArchNamespace comptime_arch_namespace = {
    .x86       = {COMPTIME_ARCH_X86, "x86", 4, false, false, true},
    .x86_64    = {COMPTIME_ARCH_X86_64, "x86_64", 8, true, false, true},
    .arm       = {COMPTIME_ARCH_ARM, "arm", 4, false, false, true},
    .aarch64   = {COMPTIME_ARCH_AARCH64, "aarch64", 8, true, false, true},
    .thumb     = {COMPTIME_ARCH_THUMB, "thumb", 4, false, true, false},
    .thumbv6m  = {COMPTIME_ARCH_THUMBV6M, "thumbv6m", 4, false, true, false},
    .thumbv7m  = {COMPTIME_ARCH_THUMBV7M, "thumbv7m", 4, false, true, false},
    .thumbv7em = {COMPTIME_ARCH_THUMBV7EM, "thumbv7em", 4, false, true, true},
    .riscv32   = {COMPTIME_ARCH_RISCV32, "riscv32", 4, false, false, true},
    .riscv64   = {COMPTIME_ARCH_RISCV64, "riscv64", 8, true, false, true},
    .avr       = {COMPTIME_ARCH_AVR, "avr", 2, false, true, false},
    .msp430    = {COMPTIME_ARCH_MSP430, "msp430", 2, false, true, false},
    .wasm32    = {COMPTIME_ARCH_WASM32, "wasm32", 4, false, false, true},
    .wasm64    = {COMPTIME_ARCH_WASM64, "wasm64", 8, true, false, true},
};

const ComptimeOSNamespace comptime_os_namespace = {
    .os_linux        = {COMPTIME_OS_LINUX, "linux", true, true, true, true, false},
    .os_windows      = {COMPTIME_OS_WINDOWS, "windows", true, true, true, true, false},
    .os_darwin       = {COMPTIME_OS_DARWIN, "darwin", true, true, true, true, false},
    .os_wasi         = {COMPTIME_OS_WASI, "wasi", true, false, false, false, false},
    .os_freestanding = {COMPTIME_OS_FREESTANDING, "freestanding", false, false, false, false, true},
    .os_zephyr       = {COMPTIME_OS_ZEPHYR, "zephyr", true, false, true, false, false},
    .os_none         = {COMPTIME_OS_NONE, "none", false, false, false, false, true},
};

const ComptimeABINamespace comptime_abi_namespace = {
    .gnu    = {COMPTIME_ABI_GNU, "gnu", true},
    .msvc   = {COMPTIME_ABI_MSVC, "msvc", true},
    .musl   = {COMPTIME_ABI_MUSL, "musl", true},
    .newlib = {COMPTIME_ABI_NEWLIB, "newlib", true},
    .bare   = {COMPTIME_ABI_BARE, "bare", false},
    .wasi   = {COMPTIME_ABI_WASI, "wasi", false},
};

const ComptimeVendorNamespace comptime_vendor_namespace = {
    .pc        = {COMPTIME_VENDOR_PC, "pc"},
    .apple     = {COMPTIME_VENDOR_APPLE, "apple"},
    .msp       = {COMPTIME_VENDOR_MSP, "msp"},
    .espressif = {COMPTIME_VENDOR_ESPRESSIF, "espressif"},
    .custom    = {COMPTIME_VENDOR_CUSTOM, "custom"},
};

const ComptimeEnvironmentNamespace comptime_environment_namespace = {
    .none   = {COMPTIME_ENVIRONMENT_NONE, "none"},
    .gnu    = {COMPTIME_ENVIRONMENT_GNU, "gnu"},
    .musl   = {COMPTIME_ENVIRONMENT_MUSL, "musl"},
    .newlib = {COMPTIME_ENVIRONMENT_NEWLIB, "newlib"},
    .wasi   = {COMPTIME_ENVIRONMENT_WASI, "wasi"},
    .bare   = {COMPTIME_ENVIRONMENT_BARE, "bare"},
};

const ComptimeEndiannessNamespace comptime_endianness_namespace = {
    .little = {COMPTIME_ENDIANNESS_LITTLE, "little"},
    .big    = {COMPTIME_ENDIANNESS_BIG, "big"},
    .mixed  = {COMPTIME_ENDIANNESS_MIXED, "mixed"},
};

const ComptimeObjectFormatNamespace comptime_object_format_namespace = {
    .elf    = {COMPTIME_OBJECT_FORMAT_ELF, "elf"},
    .pe     = {COMPTIME_OBJECT_FORMAT_PE, "pe"},
    .macho  = {COMPTIME_OBJECT_FORMAT_MACHO, "macho"},
    .wasm   = {COMPTIME_OBJECT_FORMAT_WASM, "wasm"},
    .rawbin = {COMPTIME_OBJECT_FORMAT_RAWBIN, "rawbin"},
    .hex    = {COMPTIME_OBJECT_FORMAT_HEX, "hex"},
    .uf2    = {COMPTIME_OBJECT_FORMAT_UF2, "uf2"},
};

const ComptimeBackendNamespace comptime_backend_namespace = {
    .native   = {COMPTIME_BACKEND_KIND_NATIVE, "native"},
    .wasm     = {COMPTIME_BACKEND_KIND_WASM, "wasm"},
    .embedded = {COMPTIME_BACKEND_KIND_EMBEDDED, "embedded"},
    .custom   = {COMPTIME_BACKEND_KIND_CUSTOM, "custom"},
};

const ComptimeFeatureNamespace comptime_feature_namespace = {
    .soft_float       = {COMPTIME_FEATURE_SOFT_FLOAT, "soft_float"},
    .hard_float       = {COMPTIME_FEATURE_HARD_FLOAT, "hard_float"},
    .simd             = {COMPTIME_FEATURE_SIMD, "simd"},
    .unaligned_memory = {COMPTIME_FEATURE_UNALIGNED_MEMORY, "unaligned_memory"},
    .atomics_64       = {COMPTIME_FEATURE_ATOMICS_64, "atomics_64"},
    .atomics_128      = {COMPTIME_FEATURE_ATOMICS_128, "atomics_128"},
    .threads          = {COMPTIME_FEATURE_THREADS, "threads"},
    .mmu              = {COMPTIME_FEATURE_MMU, "mmu"},
    .cache            = {COMPTIME_FEATURE_CACHE, "cache"},
    .vector_ext       = {COMPTIME_FEATURE_VECTOR_EXT, "vector_ext"},
};

void comptime_build_context_init_host(ComptimeBuildContext *ctx)
{
    if (!ctx)
        return;

    ctx->arch = comptime_arch_info(HOST_ARCH);
    ctx->os = comptime_os_info(HOST_OS);
    ctx->pointer_width = HOST_POINTER_SIZE;
    
    // Defaults for host
    ctx->abi = comptime_abi_info(COMPTIME_ABI_UNKNOWN);
    ctx->vendor = comptime_vendor_info(COMPTIME_VENDOR_UNKNOWN);
    ctx->environment = comptime_environment_info(COMPTIME_ENVIRONMENT_UNKNOWN);
    ctx->endianness = comptime_endianness_info(COMPTIME_ENDIANNESS_LITTLE); // Most hosts are little endian
    ctx->object_format = comptime_object_format_info(COMPTIME_OBJECT_FORMAT_UNKNOWN);
    ctx->backend = comptime_backend_kind_info(COMPTIME_BACKEND_KIND_NATIVE);

    // Refine defaults based on OS
    if (ctx->os.id == COMPTIME_OS_LINUX) {
        ctx->abi = comptime_abi_info(COMPTIME_ABI_GNU);
        ctx->vendor = comptime_vendor_info(COMPTIME_VENDOR_PC);
        ctx->environment = comptime_environment_info(COMPTIME_ENVIRONMENT_GNU);
        ctx->object_format = comptime_object_format_info(COMPTIME_OBJECT_FORMAT_ELF);
    } else if (ctx->os.id == COMPTIME_OS_WINDOWS) {
        ctx->abi = comptime_abi_info(COMPTIME_ABI_MSVC);
        ctx->vendor = comptime_vendor_info(COMPTIME_VENDOR_PC);
        ctx->environment = comptime_environment_info(COMPTIME_ENVIRONMENT_NONE);
        ctx->object_format = comptime_object_format_info(COMPTIME_OBJECT_FORMAT_PE);
    } else if (ctx->os.id == COMPTIME_OS_DARWIN) {
        ctx->abi = comptime_abi_info(COMPTIME_ABI_GNU); // Darwin uses GNU-like ABI usually? Or just "Darwin"?
        ctx->vendor = comptime_vendor_info(COMPTIME_VENDOR_APPLE);
        ctx->environment = comptime_environment_info(COMPTIME_ENVIRONMENT_NONE);
        ctx->object_format = comptime_object_format_info(COMPTIME_OBJECT_FORMAT_MACHO);
    }

    // Build triple from arch and os names
    static char triple_buf[256];
    snprintf(triple_buf, sizeof(triple_buf), "%s-unknown-%s", ctx->arch.name, ctx->os.name);
    ctx->triple = triple_buf;
    
    ctx->build_debug = true;
    ctx->opt_level = 0;
    ctx->mach_version = MACH_VERSION;
    ctx->build_version = "";
}

// parse target triple and initialize comptime context
void comptime_build_context_init_from_triple(ComptimeBuildContext *ctx, const char *target_triple)
{
    if (!ctx || !target_triple)
        return;

    // initialize with host defaults first
    comptime_build_context_init_host(ctx);

    // override triple
    ctx->triple = target_triple;

    ComptimeArch parsed_arch = ctx->arch.id;
    ComptimeOS parsed_os = ctx->os.id;
    unsigned char parsed_pointer_width = ctx->pointer_width;

    // parse arch from triple (arch-vendor-os)
    if (strncmp(target_triple, "x86_64", 6) == 0)
    {
        parsed_arch = COMPTIME_ARCH_X86_64;
        parsed_pointer_width = 8;
    }
    else if (strncmp(target_triple, "aarch64", 7) == 0 || strncmp(target_triple, "arm64", 5) == 0)
    {
        parsed_arch = COMPTIME_ARCH_AARCH64;
        parsed_pointer_width = 8;
    }
    else if (strncmp(target_triple, "arm", 3) == 0)
    {
        parsed_arch = COMPTIME_ARCH_ARM;
        parsed_pointer_width = 4;
    }
    else if (strncmp(target_triple, "riscv64", 7) == 0)
    {
        parsed_arch = COMPTIME_ARCH_RISCV64;
        parsed_pointer_width = 8;
    }
    else if (strncmp(target_triple, "riscv32", 7) == 0)
    {
        parsed_arch = COMPTIME_ARCH_RISCV32;
        parsed_pointer_width = 4;
    }

    // parse os from triple
    if (strstr(target_triple, "linux"))
    {
        parsed_os = COMPTIME_OS_LINUX;
    }
    else if (strstr(target_triple, "darwin") || strstr(target_triple, "macos"))
    {
        parsed_os = COMPTIME_OS_DARWIN;
    }
    else if (strstr(target_triple, "windows") || strstr(target_triple, "msvc") || strstr(target_triple, "mingw"))
    {
        parsed_os = COMPTIME_OS_WINDOWS;
    }
    else if (strstr(target_triple, "wasi"))
    {
        parsed_os = COMPTIME_OS_WASI;
    }

    // Update context with parsed values
    ctx->arch = comptime_arch_info(parsed_arch);
    ctx->os = comptime_os_info(parsed_os);
    ctx->pointer_width = parsed_pointer_width;
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

    // $mach.build.target - returns full target info
    if (strcmp(name, "mach.build.target") == 0)
    {
        // For now, we'll handle field access separately
        // This would be the full Target record access point
        return false;
    }

    // $mach.build.target.arch - returns ArchitectureInfo
    if (strcmp(name, "mach.build.target.arch") == 0)
    {
        out_value->kind = COMPTIME_ARCH_INFO;
        out_value->arch_info = ctx->arch;
        return true;
    }

    // $mach.build.target.os - returns OSInfo
    if (strcmp(name, "mach.build.target.os") == 0)
    {
        out_value->kind = COMPTIME_OS_INFO;
        out_value->os_info = ctx->os;
        return true;
    }

    // $mach.build.target.triple - returns string
    if (strcmp(name, "mach.build.target.triple") == 0)
    {
        out_value->kind = COMPTIME_STRING;
        out_value->string_val = ctx->triple;
        return true;
    }

    // $mach.build.target.pointer_width - returns u8
    if (strcmp(name, "mach.build.target.pointer_width") == 0)
    {
        out_value->kind = COMPTIME_U8;
        out_value->u8_val = ctx->pointer_width;
        return true;
    }

    // $mach.build.target.abi
    if (strcmp(name, "mach.build.target.abi") == 0)
    {
        out_value->kind = COMPTIME_ABI_INFO;
        out_value->abi_info = ctx->abi;
        return true;
    }

    // $mach.build.target.vendor
    if (strcmp(name, "mach.build.target.vendor") == 0)
    {
        out_value->kind = COMPTIME_VENDOR_INFO;
        out_value->vendor_info = ctx->vendor;
        return true;
    }

    // $mach.build.target.environment
    if (strcmp(name, "mach.build.target.environment") == 0)
    {
        out_value->kind = COMPTIME_ENVIRONMENT_INFO;
        out_value->environment_info = ctx->environment;
        return true;
    }

    // $mach.build.target.endianness
    if (strcmp(name, "mach.build.target.endianness") == 0)
    {
        out_value->kind = COMPTIME_ENDIANNESS_INFO;
        out_value->endianness_info = ctx->endianness;
        return true;
    }

    // $mach.build.target.object_format
    if (strcmp(name, "mach.build.target.object_format") == 0)
    {
        out_value->kind = COMPTIME_OBJECT_FORMAT_INFO;
        out_value->object_format_info = ctx->object_format;
        return true;
    }

    // $mach.build.target.backend
    if (strcmp(name, "mach.build.target.backend") == 0)
    {
        out_value->kind = COMPTIME_BACKEND_KIND_INFO;
        out_value->backend_kind_info = ctx->backend;
        return true;
    }

    // Nested field access for descriptors
    // $mach.build.target.arch.id, $mach.build.target.arch.name, etc.
    if (strncmp(name, "mach.build.target.arch.", 23) == 0)
    {
        const char *field = name + 23;
        if (strcmp(field, "id") == 0)
        {
            out_value->kind = COMPTIME_U8;
            out_value->u8_val = (unsigned char)ctx->arch.id;
            return true;
        }
        if (strcmp(field, "name") == 0)
        {
            out_value->kind = COMPTIME_STRING;
            out_value->string_val = ctx->arch.name;
            return true;
        }
        if (strcmp(field, "word_size") == 0)
        {
            out_value->kind = COMPTIME_U8;
            out_value->u8_val = ctx->arch.word_size;
            return true;
        }
        if (strcmp(field, "is_64bit") == 0)
        {
            out_value->kind = COMPTIME_BOOL;
            out_value->bool_val = ctx->arch.is_64bit;
            return true;
        }
        if (strcmp(field, "is_embedded") == 0)
        {
            out_value->kind = COMPTIME_BOOL;
            out_value->bool_val = ctx->arch.is_embedded;
            return true;
        }
        if (strcmp(field, "has_fpu") == 0)
        {
            out_value->kind = COMPTIME_BOOL;
            out_value->bool_val = ctx->arch.has_fpu;
            return true;
        }
    }

    // $mach.build.target.os.id, $mach.build.target.os.name, etc.
    if (strncmp(name, "mach.build.target.os.", 21) == 0)
    {
        const char *field = name + 21;
        if (strcmp(field, "id") == 0)
        {
            out_value->kind = COMPTIME_U8;
            out_value->u8_val = (unsigned char)ctx->os.id;
            return true;
        }
        if (strcmp(field, "name") == 0)
        {
            out_value->kind = COMPTIME_STRING;
            out_value->string_val = ctx->os.name;
            return true;
        }
        if (strcmp(field, "supports_filesystem") == 0)
        {
            out_value->kind = COMPTIME_BOOL;
            out_value->bool_val = ctx->os.supports_filesystem;
            return true;
        }
        if (strcmp(field, "supports_networking") == 0)
        {
            out_value->kind = COMPTIME_BOOL;
            out_value->bool_val = ctx->os.supports_networking;
            return true;
        }
        if (strcmp(field, "supports_threads") == 0)
        {
            out_value->kind = COMPTIME_BOOL;
            out_value->bool_val = ctx->os.supports_threads;
            return true;
        }
        if (strcmp(field, "has_mmu") == 0)
        {
            out_value->kind = COMPTIME_BOOL;
            out_value->bool_val = ctx->os.has_mmu;
            return true;
        }
        if (strcmp(field, "is_freestanding") == 0)
        {
            out_value->kind = COMPTIME_BOOL;
            out_value->bool_val = ctx->os.is_freestanding;
            return true;
        }
    }

    // Namespace access: $mach.os, $mach.arch, $mach.abi, etc.
    if (strcmp(name, "mach.os") == 0)
    {
        out_value->kind = COMPTIME_OS_NAMESPACE;
        out_value->os_namespace = &comptime_os_namespace;
        return true;
    }
    if (strcmp(name, "mach.arch") == 0)
    {
        out_value->kind = COMPTIME_ARCH_NAMESPACE;
        out_value->arch_namespace = &comptime_arch_namespace;
        return true;
    }
    if (strcmp(name, "mach.abi") == 0)
    {
        out_value->kind = COMPTIME_ABI_NAMESPACE;
        out_value->abi_namespace = &comptime_abi_namespace;
        return true;
    }
    if (strcmp(name, "mach.vendor") == 0)
    {
        out_value->kind = COMPTIME_VENDOR_NAMESPACE;
        out_value->vendor_namespace = &comptime_vendor_namespace;
        return true;
    }
    if (strcmp(name, "mach.environment") == 0)
    {
        out_value->kind = COMPTIME_ENVIRONMENT_NAMESPACE;
        out_value->environment_namespace = &comptime_environment_namespace;
        return true;
    }
    if (strcmp(name, "mach.endianness") == 0)
    {
        out_value->kind = COMPTIME_ENDIANNESS_NAMESPACE;
        out_value->endianness_namespace = &comptime_endianness_namespace;
        return true;
    }
    if (strcmp(name, "mach.object_format") == 0)
    {
        out_value->kind = COMPTIME_OBJECT_FORMAT_NAMESPACE;
        out_value->object_format_namespace = &comptime_object_format_namespace;
        return true;
    }
    if (strcmp(name, "mach.backend") == 0)
    {
        out_value->kind = COMPTIME_BACKEND_NAMESPACE;
        out_value->backend_namespace = &comptime_backend_namespace;
        return true;
    }
    if (strcmp(name, "mach.feature") == 0)
    {
        out_value->kind = COMPTIME_FEATURE_NAMESPACE;
        out_value->feature_namespace = &comptime_feature_namespace;
        return true;
    }

    // Namespace field access: $mach.os.linux, $mach.arch.x86_64, etc.
    if (strncmp(name, "mach.os.", 8) == 0)
    {
        const char *field = name + 8;
        if (strcmp(field, "linux") == 0) {
            out_value->kind = COMPTIME_OS_INFO;
            out_value->os_info = comptime_os_namespace.os_linux;
            return true;
        }
        if (strcmp(field, "windows") == 0) {
            out_value->kind = COMPTIME_OS_INFO;
            out_value->os_info = comptime_os_namespace.os_windows;
            return true;
        }
        if (strcmp(field, "darwin") == 0) {
            out_value->kind = COMPTIME_OS_INFO;
            out_value->os_info = comptime_os_namespace.os_darwin;
            return true;
        }
        if (strcmp(field, "wasi") == 0) {
            out_value->kind = COMPTIME_OS_INFO;
            out_value->os_info = comptime_os_namespace.os_wasi;
            return true;
        }
        if (strcmp(field, "freestanding") == 0) {
            out_value->kind = COMPTIME_OS_INFO;
            out_value->os_info = comptime_os_namespace.os_freestanding;
            return true;
        }
        if (strcmp(field, "zephyr") == 0) {
            out_value->kind = COMPTIME_OS_INFO;
            out_value->os_info = comptime_os_namespace.os_zephyr;
            return true;
        }
        if (strcmp(field, "none") == 0) {
            out_value->kind = COMPTIME_OS_INFO;
            out_value->os_info = comptime_os_namespace.os_none;
            return true;
        }
    }

    if (strncmp(name, "mach.arch.", 10) == 0)
    {
        const char *field = name + 10;
        if (strcmp(field, "x86") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.x86;
            return true;
        }
        if (strcmp(field, "x86_64") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.x86_64;
            return true;
        }
        if (strcmp(field, "arm") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.arm;
            return true;
        }
        if (strcmp(field, "aarch64") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.aarch64;
            return true;
        }
        if (strcmp(field, "thumb") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.thumb;
            return true;
        }
        if (strcmp(field, "thumbv6m") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.thumbv6m;
            return true;
        }
        if (strcmp(field, "thumbv7m") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.thumbv7m;
            return true;
        }
        if (strcmp(field, "thumbv7em") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.thumbv7em;
            return true;
        }
        if (strcmp(field, "riscv32") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.riscv32;
            return true;
        }
        if (strcmp(field, "riscv64") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.riscv64;
            return true;
        }
        if (strcmp(field, "avr") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.avr;
            return true;
        }
        if (strcmp(field, "msp430") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.msp430;
            return true;
        }
        if (strcmp(field, "wasm32") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.wasm32;
            return true;
        }
        if (strcmp(field, "wasm64") == 0) {
            out_value->kind = COMPTIME_ARCH_INFO;
            out_value->arch_info = comptime_arch_namespace.wasm64;
            return true;
        }
    }

    if (strncmp(name, "mach.abi.", 9) == 0)
    {
        const char *field = name + 9;
        if (strcmp(field, "gnu") == 0) {
            out_value->kind = COMPTIME_ABI_INFO;
            out_value->abi_info = comptime_abi_namespace.gnu;
            return true;
        }
        if (strcmp(field, "msvc") == 0) {
            out_value->kind = COMPTIME_ABI_INFO;
            out_value->abi_info = comptime_abi_namespace.msvc;
            return true;
        }
        if (strcmp(field, "musl") == 0) {
            out_value->kind = COMPTIME_ABI_INFO;
            out_value->abi_info = comptime_abi_namespace.musl;
            return true;
        }
        if (strcmp(field, "newlib") == 0) {
            out_value->kind = COMPTIME_ABI_INFO;
            out_value->abi_info = comptime_abi_namespace.newlib;
            return true;
        }
        if (strcmp(field, "bare") == 0) {
            out_value->kind = COMPTIME_ABI_INFO;
            out_value->abi_info = comptime_abi_namespace.bare;
            return true;
        }
        if (strcmp(field, "wasi") == 0) {
            out_value->kind = COMPTIME_ABI_INFO;
            out_value->abi_info = comptime_abi_namespace.wasi;
            return true;
        }
    }

    if (strncmp(name, "mach.vendor.", 12) == 0)
    {
        const char *field = name + 12;
        if (strcmp(field, "pc") == 0) {
            out_value->kind = COMPTIME_VENDOR_INFO;
            out_value->vendor_info = comptime_vendor_namespace.pc;
            return true;
        }
        if (strcmp(field, "apple") == 0) {
            out_value->kind = COMPTIME_VENDOR_INFO;
            out_value->vendor_info = comptime_vendor_namespace.apple;
            return true;
        }
        if (strcmp(field, "msp") == 0) {
            out_value->kind = COMPTIME_VENDOR_INFO;
            out_value->vendor_info = comptime_vendor_namespace.msp;
            return true;
        }
        if (strcmp(field, "espressif") == 0) {
            out_value->kind = COMPTIME_VENDOR_INFO;
            out_value->vendor_info = comptime_vendor_namespace.espressif;
            return true;
        }
        if (strcmp(field, "custom") == 0) {
            out_value->kind = COMPTIME_VENDOR_INFO;
            out_value->vendor_info = comptime_vendor_namespace.custom;
            return true;
        }
    }

    if (strncmp(name, "mach.environment.", 17) == 0)
    {
        const char *field = name + 17;
        if (strcmp(field, "none") == 0) {
            out_value->kind = COMPTIME_ENVIRONMENT_INFO;
            out_value->environment_info = comptime_environment_namespace.none;
            return true;
        }
        if (strcmp(field, "gnu") == 0) {
            out_value->kind = COMPTIME_ENVIRONMENT_INFO;
            out_value->environment_info = comptime_environment_namespace.gnu;
            return true;
        }
        if (strcmp(field, "musl") == 0) {
            out_value->kind = COMPTIME_ENVIRONMENT_INFO;
            out_value->environment_info = comptime_environment_namespace.musl;
            return true;
        }
        if (strcmp(field, "newlib") == 0) {
            out_value->kind = COMPTIME_ENVIRONMENT_INFO;
            out_value->environment_info = comptime_environment_namespace.newlib;
            return true;
        }
        if (strcmp(field, "wasi") == 0) {
            out_value->kind = COMPTIME_ENVIRONMENT_INFO;
            out_value->environment_info = comptime_environment_namespace.wasi;
            return true;
        }
        if (strcmp(field, "bare") == 0) {
            out_value->kind = COMPTIME_ENVIRONMENT_INFO;
            out_value->environment_info = comptime_environment_namespace.bare;
            return true;
        }
    }

    if (strncmp(name, "mach.endianness.", 16) == 0)
    {
        const char *field = name + 16;
        if (strcmp(field, "little") == 0) {
            out_value->kind = COMPTIME_ENDIANNESS_INFO;
            out_value->endianness_info = comptime_endianness_namespace.little;
            return true;
        }
        if (strcmp(field, "big") == 0) {
            out_value->kind = COMPTIME_ENDIANNESS_INFO;
            out_value->endianness_info = comptime_endianness_namespace.big;
            return true;
        }
        if (strcmp(field, "mixed") == 0) {
            out_value->kind = COMPTIME_ENDIANNESS_INFO;
            out_value->endianness_info = comptime_endianness_namespace.mixed;
            return true;
        }
    }

    if (strncmp(name, "mach.object_format.", 19) == 0)
    {
        const char *field = name + 19;
        if (strcmp(field, "elf") == 0) {
            out_value->kind = COMPTIME_OBJECT_FORMAT_INFO;
            out_value->object_format_info = comptime_object_format_namespace.elf;
            return true;
        }
        if (strcmp(field, "pe") == 0) {
            out_value->kind = COMPTIME_OBJECT_FORMAT_INFO;
            out_value->object_format_info = comptime_object_format_namespace.pe;
            return true;
        }
        if (strcmp(field, "macho") == 0) {
            out_value->kind = COMPTIME_OBJECT_FORMAT_INFO;
            out_value->object_format_info = comptime_object_format_namespace.macho;
            return true;
        }
        if (strcmp(field, "wasm") == 0) {
            out_value->kind = COMPTIME_OBJECT_FORMAT_INFO;
            out_value->object_format_info = comptime_object_format_namespace.wasm;
            return true;
        }
        if (strcmp(field, "rawbin") == 0) {
            out_value->kind = COMPTIME_OBJECT_FORMAT_INFO;
            out_value->object_format_info = comptime_object_format_namespace.rawbin;
            return true;
        }
        if (strcmp(field, "hex") == 0) {
            out_value->kind = COMPTIME_OBJECT_FORMAT_INFO;
            out_value->object_format_info = comptime_object_format_namespace.hex;
            return true;
        }
        if (strcmp(field, "uf2") == 0) {
            out_value->kind = COMPTIME_OBJECT_FORMAT_INFO;
            out_value->object_format_info = comptime_object_format_namespace.uf2;
            return true;
        }
    }

    if (strncmp(name, "mach.backend.", 13) == 0)
    {
        const char *field = name + 13;
        if (strcmp(field, "native") == 0) {
            out_value->kind = COMPTIME_BACKEND_KIND_INFO;
            out_value->backend_kind_info = comptime_backend_namespace.native;
            return true;
        }
        if (strcmp(field, "wasm") == 0) {
            out_value->kind = COMPTIME_BACKEND_KIND_INFO;
            out_value->backend_kind_info = comptime_backend_namespace.wasm;
            return true;
        }
        if (strcmp(field, "embedded") == 0) {
            out_value->kind = COMPTIME_BACKEND_KIND_INFO;
            out_value->backend_kind_info = comptime_backend_namespace.embedded;
            return true;
        }
        if (strcmp(field, "custom") == 0) {
            out_value->kind = COMPTIME_BACKEND_KIND_INFO;
            out_value->backend_kind_info = comptime_backend_namespace.custom;
            return true;
        }
    }

    if (strncmp(name, "mach.feature.", 13) == 0)
    {
        const char *field = name + 13;
        if (strcmp(field, "soft_float") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.soft_float;
            return true;
        }
        if (strcmp(field, "hard_float") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.hard_float;
            return true;
        }
        if (strcmp(field, "simd") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.simd;
            return true;
        }
        if (strcmp(field, "unaligned_memory") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.unaligned_memory;
            return true;
        }
        if (strcmp(field, "atomics_64") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.atomics_64;
            return true;
        }
        if (strcmp(field, "atomics_128") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.atomics_128;
            return true;
        }
        if (strcmp(field, "threads") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.threads;
            return true;
        }
        if (strcmp(field, "mmu") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.mmu;
            return true;
        }
        if (strcmp(field, "cache") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.cache;
            return true;
        }
        if (strcmp(field, "vector_ext") == 0) {
            out_value->kind = COMPTIME_FEATURE_INFO;
            out_value->feature_info = comptime_feature_namespace.vector_ext;
            return true;
        }
    }

    // Build configuration (unchanged)
    if (strcmp(name, "mach.build.debug") == 0)
    {
        out_value->kind = COMPTIME_BOOL;
        out_value->bool_val = ctx->build_debug;
        return true;
    }
    if (strcmp(name, "mach.build.version") == 0)
    {
        out_value->kind = COMPTIME_STRING;
        out_value->string_val = ctx->build_version ? ctx->build_version : "";
        return true;
    }

    // Mach language information (unchanged)
    if (strcmp(name, "mach.version") == 0)
    {
        out_value->kind = COMPTIME_STRING;
        out_value->string_val = ctx->mach_version;
        return true;
    }

    return false;
}
