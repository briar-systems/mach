#ifndef TARGET_H
#define TARGET_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    ARCH_UNKNOWN,

    ARCH_X86,    // 32-bit x86
    ARCH_X86_64, // 64-bit x86_64
} Architecture;

typedef enum
{
    OS_UNKNOWN,

    OS_LINUX,   // Linux
    OS_WINDOWS, // Windows
    OS_MACOS,   // macOS
} OS;

typedef enum
{
    VENDOR_UNKNOWN,

    VENDOR_INTEL, // Intel
    VENDOR_AMD,   // AMD
} Vendor;

typedef enum
{
    FORMAT_UNKNOWN,

    FORMAT_ELF,   // Executable and Linkable Format (Unix/Linux)
    FORMAT_COFF,  // Common Object File Format
    FORMAT_PE,    // Portable Executable (Windows)
    FORMAT_MACHO, // Mach Object (macOS/iOS)
} Format;

typedef enum
{
    ENDIAN_UNKNOWN,

    ENDIAN_LITTLE,
    ENDIAN_BIG
} Endian;

typedef enum
{
    CALL_UNKNOWN,

    CALL_DEFAULT,    // Default for architecture/OS
    CALL_CDECL,      // C declaration (x86)
    CALL_STDCALL,    // Standard call (Windows x86)
    CALL_FASTCALL,   // Fast call (registers first)
    CALL_VECTORCALL, // Vector call (SIMD)
    CALL_REGCALL,    // Register call (maximize registers)
    CALL_THISCALL,   // this pointer in register (C++)
    CALL_AAPCS,      // ARM procedure call standard
    CALL_WIN64,      // Windows x64 calling convention
    CALL_SYSV        // System V AMD64 ENV
} CallingConvention;

typedef struct
{
    unsigned mmx : 1;         // MMX Instructions
    unsigned sse : 1;         // SSE
    unsigned sse2 : 1;        // SSE 2
    unsigned sse3 : 1;        // SSE 3
    unsigned ssse3 : 1;       // Supplemental SSE 3
    unsigned sse4_1 : 1;      // SSE 4.1
    unsigned sse4_2 : 1;      // SSE 4.2
    unsigned avx : 1;         // Advanced Vector Extensions
    unsigned avx2 : 1;        // AVX 2
    unsigned avx512f : 1;     // AVX-512 Foundation
    unsigned fma : 1;         // Fused Multiply-Add
    unsigned popcnt : 1;      // Population Count
    unsigned aes : 1;         // AES Instruction Set
    unsigned pclmul : 1;      // PCLMULQDQ Instruction
    unsigned rdrand : 1;      // RDRAND Instruction
    unsigned rdseed : 1;      // RDSEED Instruction
    unsigned bmi1 : 1;        // Bit Manipulation Instruction Set 1
    unsigned bmi2 : 1;        // Bit Manipulation Instruction Set 2
    unsigned adx : 1;         // Multi-Precision Add-Carry Instructions
    unsigned sha : 1;         // SHA Extensions
    unsigned prefetchwt1 : 1; // PREFETCHWT1 Instruction
    unsigned rtm : 1;         // Restricted Transactional Memory
} X86Features;

typedef union
{
    X86Features x86; // x86/x64 features
    uint64_t raw;
} Features;

typedef enum
{
    OPT_NONE, // No optimization
} OptLevel;

typedef struct
{
    bool pic;      // Position Independent Code
    bool pie;      // Position Independent Executable
    bool lto;      // Link-Time Optimization
    bool sanitize; // Address Sanitizer
    bool debug;    // Include debug information

    OptLevel opt_level;
} CodegenOptions;

typedef struct Target
{
    Architecture arch;
    Vendor vendor;
    OS os;

    Endian endian;               // Byte order
    Format obj_format;           // Object file format
    Features cpu_features;       // CPU-specific features
    CallingConvention call_conv; // Calling convention

    uint8_t ptr_size;        // Pointer size (typically 4 or 8)
    uint8_t word_size;       // Word size (typically 4 or 8)
    uint8_t stack_alignment; // Stack alignment requirement
    uint8_t data_alignment;  // Data alignment requirement

    CodegenOptions options; // Code generation options
} Target;

Target target_from_triple(const char *triple);
char *target_to_triple(Target target);

Target target_host();
Target target_default(OS os);

bool target_is_valid(Target target);

const char *architecture_to_string(Architecture arch);
const char *os_to_string(OS os);
const char *vendor_to_string(Vendor vendor);
const char *format_to_string(Format format);
const char *endian_to_string(Endian endian);
const char *calling_convention_to_string(CallingConvention cc);

Architecture architecture_from_string(const char *str);
OS os_from_string(const char *str);
Vendor vendor_from_string(const char *str);
Format format_from_string(const char *str);
Endian endian_from_string(const char *str);
CallingConvention calling_convention_from_string(const char *str);

void target_init_defaults(Target *target, Architecture arch);

Architecture detect_host_architecture();
Vendor detect_host_vendor();
OS detect_host_os();
Endian detect_host_endianness();
void detect_host_cpu_features(Features *features);
Format detect_host_format();
CallingConvention detect_host_calling_convention();

#endif // TARGET_H
