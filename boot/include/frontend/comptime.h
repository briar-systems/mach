#ifndef COMPTIME_H
#define COMPTIME_H

#include <stdbool.h>

// OS enum matching target.mach
typedef enum ComptimeOS
{
    COMPTIME_OS_UNKNOWN      = 0,
    COMPTIME_OS_LINUX        = 1,
    COMPTIME_OS_WINDOWS      = 2,
    COMPTIME_OS_DARWIN       = 3,
    COMPTIME_OS_WASI         = 4,
    COMPTIME_OS_FREESTANDING = 5,
    COMPTIME_OS_ZEPHYR       = 6,
    COMPTIME_OS_NONE         = 7,
} ComptimeOS;

// Architecture enum matching target.mach
typedef enum ComptimeArch
{
    COMPTIME_ARCH_UNKNOWN   = 0,
    COMPTIME_ARCH_X86       = 1,
    COMPTIME_ARCH_X86_64    = 2,
    COMPTIME_ARCH_ARM       = 3,
    COMPTIME_ARCH_AARCH64   = 4,
    COMPTIME_ARCH_THUMB     = 5,
    COMPTIME_ARCH_THUMBV6M  = 6,
    COMPTIME_ARCH_THUMBV7M  = 7,
    COMPTIME_ARCH_THUMBV7EM = 8,
    COMPTIME_ARCH_RISCV32   = 9,
    COMPTIME_ARCH_RISCV64   = 10,
    COMPTIME_ARCH_AVR       = 11,
    COMPTIME_ARCH_MSP430    = 12,
    COMPTIME_ARCH_WASM32    = 13,
    COMPTIME_ARCH_WASM64    = 14,
} ComptimeArch;

// ABI enum matching target.mach
typedef enum ComptimeABI
{
    COMPTIME_ABI_UNKNOWN = 0,
    COMPTIME_ABI_GNU     = 1,
    COMPTIME_ABI_MSVC    = 2,
    COMPTIME_ABI_MUSL    = 3,
    COMPTIME_ABI_NEWLIB  = 4,
    COMPTIME_ABI_BARE    = 5,
    COMPTIME_ABI_WASI    = 6,
} ComptimeABI;

// Vendor enum matching target.mach
typedef enum ComptimeVendor
{
    COMPTIME_VENDOR_UNKNOWN   = 0,
    COMPTIME_VENDOR_PC        = 1,
    COMPTIME_VENDOR_APPLE     = 2,
    COMPTIME_VENDOR_MSP       = 3,
    COMPTIME_VENDOR_ESPRESSIF = 4,
    COMPTIME_VENDOR_CUSTOM    = 5,
} ComptimeVendor;

// Environment enum matching target.mach
typedef enum ComptimeEnvironment
{
    COMPTIME_ENVIRONMENT_UNKNOWN = 0,
    COMPTIME_ENVIRONMENT_NONE    = 1,
    COMPTIME_ENVIRONMENT_GNU     = 2,
    COMPTIME_ENVIRONMENT_MUSL    = 3,
    COMPTIME_ENVIRONMENT_NEWLIB  = 4,
    COMPTIME_ENVIRONMENT_WASI    = 5,
    COMPTIME_ENVIRONMENT_BARE    = 6,
} ComptimeEnvironment;

// Endianness enum matching target.mach
typedef enum ComptimeEndianness
{
    COMPTIME_ENDIANNESS_UNKNOWN = 0,
    COMPTIME_ENDIANNESS_LITTLE  = 1,
    COMPTIME_ENDIANNESS_BIG     = 2,
    COMPTIME_ENDIANNESS_MIXED   = 3,
} ComptimeEndianness;

// ObjectFormat enum matching target.mach
typedef enum ComptimeObjectFormat
{
    COMPTIME_OBJECT_FORMAT_UNKNOWN = 0,
    COMPTIME_OBJECT_FORMAT_ELF     = 1,
    COMPTIME_OBJECT_FORMAT_PE      = 2,
    COMPTIME_OBJECT_FORMAT_MACHO   = 3,
    COMPTIME_OBJECT_FORMAT_WASM    = 4,
    COMPTIME_OBJECT_FORMAT_RAWBIN  = 5,
    COMPTIME_OBJECT_FORMAT_HEX     = 6,
    COMPTIME_OBJECT_FORMAT_UF2     = 7,
} ComptimeObjectFormat;

// BackendKind enum matching target.mach
typedef enum ComptimeBackendKind
{
    COMPTIME_BACKEND_KIND_UNKNOWN  = 0,
    COMPTIME_BACKEND_KIND_NATIVE   = 1,
    COMPTIME_BACKEND_KIND_WASM     = 2,
    COMPTIME_BACKEND_KIND_EMBEDDED = 3,
    COMPTIME_BACKEND_KIND_CUSTOM   = 4,
} ComptimeBackendKind;

// Feature enum matching target.mach
typedef enum ComptimeFeature
{
    COMPTIME_FEATURE_SOFT_FLOAT       = 0,
    COMPTIME_FEATURE_HARD_FLOAT       = 1,
    COMPTIME_FEATURE_SIMD             = 2,
    COMPTIME_FEATURE_UNALIGNED_MEMORY = 3,
    COMPTIME_FEATURE_ATOMICS_64       = 4,
    COMPTIME_FEATURE_ATOMICS_128      = 5,
    COMPTIME_FEATURE_THREADS          = 6,
    COMPTIME_FEATURE_MMU              = 7,
    COMPTIME_FEATURE_CACHE            = 8,
    COMPTIME_FEATURE_VECTOR_EXT       = 9,
} ComptimeFeature;

// Info descriptor structures (simplified for bootstrap)
typedef struct ComptimeArchitectureInfo
{
    ComptimeArch id;
    const char  *name;
    unsigned char word_size;
    bool is_64bit;
    bool is_embedded;
    bool has_fpu;
} ComptimeArchitectureInfo;

typedef struct ComptimeOSInfo
{
    ComptimeOS  id;
    const char *name;
    bool supports_filesystem;
    bool supports_networking;
    bool supports_threads;
    bool has_mmu;
    bool is_freestanding;
} ComptimeOSInfo;

typedef struct ComptimeABIInfo
{
    ComptimeABI id;
    const char *name;
    bool requires_libc;
} ComptimeABIInfo;

typedef struct ComptimeVendorInfo
{
    ComptimeVendor id;
    const char *name;
} ComptimeVendorInfo;

typedef struct ComptimeEnvironmentInfo
{
    ComptimeEnvironment id;
    const char *name;
} ComptimeEnvironmentInfo;

typedef struct ComptimeEndiannessInfo
{
    ComptimeEndianness id;
    const char *name;
} ComptimeEndiannessInfo;

typedef struct ComptimeObjectFormatInfo
{
    ComptimeObjectFormat id;
    const char *name;
} ComptimeObjectFormatInfo;

typedef struct ComptimeBackendKindInfo
{
    ComptimeBackendKind id;
    const char *name;
} ComptimeBackendKindInfo;

typedef struct ComptimeFeatureInfo
{
    ComptimeFeature id;
    const char *name;
} ComptimeFeatureInfo;

// Namespace structures matching target.mach
typedef struct ComptimeArchNamespace
{
    ComptimeArchitectureInfo x86;
    ComptimeArchitectureInfo x86_64;
    ComptimeArchitectureInfo arm;
    ComptimeArchitectureInfo aarch64;
    ComptimeArchitectureInfo thumb;
    ComptimeArchitectureInfo thumbv6m;
    ComptimeArchitectureInfo thumbv7m;
    ComptimeArchitectureInfo thumbv7em;
    ComptimeArchitectureInfo riscv32;
    ComptimeArchitectureInfo riscv64;
    ComptimeArchitectureInfo avr;
    ComptimeArchitectureInfo msp430;
    ComptimeArchitectureInfo wasm32;
    ComptimeArchitectureInfo wasm64;
} ComptimeArchNamespace;

typedef struct ComptimeOSNamespace
{
    ComptimeOSInfo os_linux;
    ComptimeOSInfo os_windows;
    ComptimeOSInfo os_darwin;
    ComptimeOSInfo os_wasi;
    ComptimeOSInfo os_freestanding;
    ComptimeOSInfo os_zephyr;
    ComptimeOSInfo os_none;
} ComptimeOSNamespace;

typedef struct ComptimeABINamespace
{
    ComptimeABIInfo gnu;
    ComptimeABIInfo msvc;
    ComptimeABIInfo musl;
    ComptimeABIInfo newlib;
    ComptimeABIInfo bare;
    ComptimeABIInfo wasi;
} ComptimeABINamespace;

typedef struct ComptimeVendorNamespace
{
    ComptimeVendorInfo pc;
    ComptimeVendorInfo apple;
    ComptimeVendorInfo msp;
    ComptimeVendorInfo espressif;
    ComptimeVendorInfo custom;
} ComptimeVendorNamespace;

typedef struct ComptimeEnvironmentNamespace
{
    ComptimeEnvironmentInfo none;
    ComptimeEnvironmentInfo gnu;
    ComptimeEnvironmentInfo musl;
    ComptimeEnvironmentInfo newlib;
    ComptimeEnvironmentInfo wasi;
    ComptimeEnvironmentInfo bare;
} ComptimeEnvironmentNamespace;

typedef struct ComptimeEndiannessNamespace
{
    ComptimeEndiannessInfo little;
    ComptimeEndiannessInfo big;
    ComptimeEndiannessInfo mixed;
} ComptimeEndiannessNamespace;

typedef struct ComptimeObjectFormatNamespace
{
    ComptimeObjectFormatInfo elf;
    ComptimeObjectFormatInfo pe;
    ComptimeObjectFormatInfo macho;
    ComptimeObjectFormatInfo wasm;
    ComptimeObjectFormatInfo rawbin;
    ComptimeObjectFormatInfo hex;
    ComptimeObjectFormatInfo uf2;
} ComptimeObjectFormatNamespace;

typedef struct ComptimeBackendNamespace
{
    ComptimeBackendKindInfo native;
    ComptimeBackendKindInfo wasm;
    ComptimeBackendKindInfo embedded;
    ComptimeBackendKindInfo custom;
} ComptimeBackendNamespace;

typedef struct ComptimeFeatureNamespace
{
    ComptimeFeatureInfo soft_float;
    ComptimeFeatureInfo hard_float;
    ComptimeFeatureInfo simd;
    ComptimeFeatureInfo unaligned_memory;
    ComptimeFeatureInfo atomics_64;
    ComptimeFeatureInfo atomics_128;
    ComptimeFeatureInfo threads;
    ComptimeFeatureInfo mmu;
    ComptimeFeatureInfo cache;
    ComptimeFeatureInfo vector_ext;
} ComptimeFeatureNamespace;

typedef struct ComptimeBuildContext
{
    // Target information (now using Info descriptors)
    ComptimeArchitectureInfo arch;
    ComptimeOSInfo           os;
    ComptimeABIInfo          abi;
    ComptimeVendorInfo       vendor;
    ComptimeEnvironmentInfo  environment;
    ComptimeEndiannessInfo   endianness;
    ComptimeObjectFormatInfo object_format;
    ComptimeBackendKindInfo  backend;
    
    const char              *triple;
    unsigned char            pointer_width;

    // Build configuration (unchanged)
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
    COMPTIME_BOOL,
    COMPTIME_ARCH_INFO,
    COMPTIME_OS_INFO,
    COMPTIME_ABI_INFO,
    COMPTIME_VENDOR_INFO,
    COMPTIME_ENVIRONMENT_INFO,
    COMPTIME_ENDIANNESS_INFO,
    COMPTIME_OBJECT_FORMAT_INFO,
    COMPTIME_BACKEND_KIND_INFO,
    COMPTIME_FEATURE_INFO,
    COMPTIME_ARCH_NAMESPACE,
    COMPTIME_OS_NAMESPACE,
    COMPTIME_ABI_NAMESPACE,
    COMPTIME_VENDOR_NAMESPACE,
    COMPTIME_ENVIRONMENT_NAMESPACE,
    COMPTIME_ENDIANNESS_NAMESPACE,
    COMPTIME_OBJECT_FORMAT_NAMESPACE,
    COMPTIME_BACKEND_NAMESPACE,
    COMPTIME_FEATURE_NAMESPACE,
} ComptimeValueKind;

typedef struct ComptimeValue
{
    ComptimeValueKind kind;
    union
    {
        unsigned char u8_val;
        unsigned long u64_val;
        const char   *string_val;
        bool          bool_val;
        ComptimeArchitectureInfo arch_info;
        ComptimeOSInfo           os_info;
        ComptimeABIInfo          abi_info;
        ComptimeVendorInfo       vendor_info;
        ComptimeEnvironmentInfo  environment_info;
        ComptimeEndiannessInfo   endianness_info;
        ComptimeObjectFormatInfo object_format_info;
        ComptimeBackendKindInfo  backend_kind_info;
        ComptimeFeatureInfo      feature_info;
        const ComptimeArchNamespace       *arch_namespace;
        const ComptimeOSNamespace         *os_namespace;
        const ComptimeABINamespace        *abi_namespace;
        const ComptimeVendorNamespace     *vendor_namespace;
        const ComptimeEnvironmentNamespace *environment_namespace;
        const ComptimeEndiannessNamespace  *endianness_namespace;
        const ComptimeObjectFormatNamespace *object_format_namespace;
        const ComptimeBackendNamespace     *backend_namespace;
        const ComptimeFeatureNamespace     *feature_namespace;
    };
} ComptimeValue;

// Helper functions for creating descriptors
ComptimeArchitectureInfo comptime_arch_info(ComptimeArch arch);
ComptimeOSInfo comptime_os_info(ComptimeOS os);
ComptimeABIInfo comptime_abi_info(ComptimeABI abi);
ComptimeVendorInfo comptime_vendor_info(ComptimeVendor vendor);
ComptimeEnvironmentInfo comptime_environment_info(ComptimeEnvironment env);
ComptimeEndiannessInfo comptime_endianness_info(ComptimeEndianness endian);
ComptimeObjectFormatInfo comptime_object_format_info(ComptimeObjectFormat fmt);
ComptimeBackendKindInfo comptime_backend_kind_info(ComptimeBackendKind kind);
ComptimeFeatureInfo comptime_feature_info(ComptimeFeature feat);

// Global namespace instances
extern const ComptimeArchNamespace comptime_arch_namespace;
extern const ComptimeOSNamespace comptime_os_namespace;
extern const ComptimeABINamespace comptime_abi_namespace;
extern const ComptimeVendorNamespace comptime_vendor_namespace;
extern const ComptimeEnvironmentNamespace comptime_environment_namespace;
extern const ComptimeEndiannessNamespace comptime_endianness_namespace;
extern const ComptimeObjectFormatNamespace comptime_object_format_namespace;
extern const ComptimeBackendNamespace comptime_backend_namespace;
extern const ComptimeFeatureNamespace comptime_feature_namespace;

void comptime_build_context_init_host(ComptimeBuildContext *ctx);
void comptime_build_context_init_from_triple(ComptimeBuildContext *ctx, const char *target_triple);
bool comptime_get_constant(const char *name, ComptimeValue *out_value, const ComptimeBuildContext *ctx);

#endif // COMPTIME_H
