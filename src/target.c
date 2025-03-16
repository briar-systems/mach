#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "target.h"

Target target_from_triple(const char *triple)
{
    Target target;
    memset(&target, 0, sizeof(Target));

    target.arch = ARCH_UNKNOWN;
    target.vendor = VENDOR_UNKNOWN;
    target.os = OS_UNKNOWN;

    char *copy = strdup(triple);
    if (!copy)
        return target;

    char *arch_str = strtok(copy, "-");
    char *vendor_str = strtok(NULL, "-");
    char *os_str = strtok(NULL, "-");
    char *env_str = strtok(NULL, "-");

    if (arch_str)
    {
        target.arch = architecture_from_string(arch_str);
    }
    if (vendor_str)
    {
        target.vendor = vendor_from_string(vendor_str);
    }
    if (os_str)
    {
        target.os = os_from_string(os_str);
    }

    target_init_defaults(&target, target.arch);

    free(copy);
    return target;
}

char *target_to_triple(Target target)
{
    const char *arch = architecture_to_string(target.arch);
    const char *vendor = vendor_to_string(target.vendor);
    const char *os = os_to_string(target.os);

    int len = strlen(arch) + strlen(vendor) + strlen(os) + 4; // 3 dashes and null terminator

    char *triple = malloc(len);
    if (!triple)
    {
        fprintf(stderr, "failed to allocate memory for target triple");
        return NULL;
    }

    snprintf((char *)triple, len, "%s-%s-%s", arch, vendor, os);

    return triple;
}

Target target_host()
{
    Target target;
    memset(&target, 0, sizeof(Target));

    target.arch = detect_host_architecture();
    target.vendor = detect_host_vendor();
    target.os = detect_host_os();
    target.endian = detect_host_endianness();
    target.obj_format = detect_host_format();
    detect_host_cpu_features(&target.cpu_features);

    target_init_defaults(&target, target.arch);

    return target;
}

bool target_is_valid(Target target)
{
    if (target.arch == ARCH_UNKNOWN || target.vendor == VENDOR_UNKNOWN || target.os == OS_UNKNOWN)
    {
        return false;
    }

    return true;
}

const char *architecture_to_string(Architecture arch)
{
    switch (arch)
    {
    case ARCH_X86:
        return "x86";
    case ARCH_X86_64:
        return "x86_64";
    default:
        return "unknown";
    }
}

const char *os_to_string(OS os)
{
    switch (os)
    {
    case OS_LINUX:
        return "linux";
    case OS_WINDOWS:
        return "windows";
    case OS_MACOS:
        return "macos";
    default:
        return "unknown";
    }
}

const char *vendor_to_string(Vendor vendor)
{
    switch (vendor)
    {
    case VENDOR_INTEL:
        return "intel";
    case VENDOR_AMD:
        return "amd";
    default:
        return "unknown";
    }
}

const char *format_to_string(Format format)
{
    switch (format)
    {
    case FORMAT_ELF:
        return "elf";
    case FORMAT_PE:
        return "pe";
    case FORMAT_MACHO:
        return "macho";
    default:
        return "unknown";
    }
}

const char *endian_to_string(Endian endian)
{
    switch (endian)
    {
    case ENDIAN_LITTLE:
        return "little";
    case ENDIAN_BIG:
        return "big";
    default:
        return "unknown";
    }
}

const char *calling_convention_to_string(CallingConvention cc)
{
    switch (cc)
    {
    case CALL_SYSV:
        return "sysv";
    case CALL_WIN64:
        return "win64";
    default:
        return "unknown";
    }
}

Architecture architecture_from_string(const char *str)
{
    if (strcmp(str, "x86") == 0)
        return ARCH_X86;
    else if (strcmp(str, "x86_64") == 0)
        return ARCH_X86_64;
    else
        return ARCH_UNKNOWN;
}

OS os_from_string(const char *str)
{
    if (strcmp(str, "linux") == 0)
        return OS_LINUX;
    else if (strcmp(str, "windows") == 0)
        return OS_WINDOWS;
    else if (strcmp(str, "macos") == 0)
        return OS_MACOS;
    else
        return OS_UNKNOWN;
}

Vendor vendor_from_string(const char *str)
{
    if (strcmp(str, "intel") == 0)
        return VENDOR_INTEL;
    else if (strcmp(str, "amd") == 0)
        return VENDOR_AMD;
    else
        return VENDOR_UNKNOWN;
}

Format format_from_string(const char *str)
{
    if (strcmp(str, "elf") == 0)
        return FORMAT_ELF;
    else if (strcmp(str, "pe") == 0)
        return FORMAT_PE;
    else if (strcmp(str, "macho") == 0)
        return FORMAT_MACHO;
    else
        return FORMAT_UNKNOWN;
}

Endian endian_from_string(const char *str)
{
    if (strcmp(str, "little") == 0)
        return ENDIAN_LITTLE;
    else if (strcmp(str, "big") == 0)
        return ENDIAN_BIG;
    else
        return ENDIAN_UNKNOWN;
}

CallingConvention calling_convention_from_string(const char *str)
{
    if (strcmp(str, "sysv") == 0)
        return CALL_SYSV;
    else if (strcmp(str, "win64") == 0)
        return CALL_WIN64;
    else
        return CALL_UNKNOWN;
}

void target_init_defaults(Target *target, Architecture arch)
{
    switch (arch)
    {
    case ARCH_X86:
        target->ptr_size = 4;
        target->word_size = 4;
        target->stack_alignment = 4;
        target->data_alignment = 4;
        target->endian = ENDIAN_LITTLE;
        break;
    case ARCH_X86_64:
        target->ptr_size = 8;
        target->word_size = 8;
        target->stack_alignment = 16;
        target->data_alignment = 8;
        target->endian = ENDIAN_LITTLE;
        break;
    default:
        target->ptr_size = 8;
        target->word_size = 8;
        target->stack_alignment = 16;
        target->data_alignment = 8;
        target->endian = ENDIAN_LITTLE;
        break;
    }

    switch (target->os)
    {
    case OS_LINUX:
        target->obj_format = FORMAT_ELF;
        if (target->arch == ARCH_X86_64)
        {
            target->call_conv = CALL_SYSV;
        }
        break;
    default:
        target->obj_format = FORMAT_UNKNOWN;
        target->call_conv = CALL_UNKNOWN;
        break;
    }
}

Architecture detect_host_architecture()
{
#if defined(__x86_64__) || defined(_M_X64)
    return ARCH_X86_64;
#elif defined(__i386__) || defined(_M_IX86)
    return ARCH_X86;
#endif

    return ARCH_UNKNOWN;
}

Vendor detect_host_vendor()
{
#if defined(__INTEL_COMPILER) || defined(_INTEL_COMPILER) || defined(_INTEL_LLVM_COMPILER)
    return VENDOR_INTEL;
#elif defined(__AMD__) || defined(_AMD_)
    return VENDOR_AMD;
#else
    unsigned int regs[4];

#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("cpuid" : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3]) : "a"(0));
#endif

    char vendor[13] = {0};
    memcpy(vendor, &regs[1], 4);
    memcpy(vendor + 4, &regs[3], 4);
    memcpy(vendor + 8, &regs[2], 4);

    if (strcmp(vendor, "GenuineIntel") == 0)
    {
        return VENDOR_INTEL;
    }
    else if (strcmp(vendor, "AuthenticAMD") == 0)
    {
        return VENDOR_AMD;
    }

    return VENDOR_UNKNOWN;
#endif
}

OS detect_host_os()
{
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    return OS_WINDOWS;
#elif defined(__APPLE__) && defined(__MACH__)
    // Could further distinguish iOS vs macOS if needed with:
    // #include <TargetConditionals.h>
    // #if TARGET_OS_IPHONE
    //     return OS_IOS;
    // #else
    return OS_MACOS;
    // #endif
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    return OS_LINUX;
#else
    // Fallback to your existing runtime detection
    // check for Windows
    if (fopen("C:\\Windows\\System32\\kernel32.dll", "rb") != NULL)
    {
        return OS_WINDOWS;
    }

    // check for common Unix/Linux indicators
    if (fopen("/proc/version", "r") != NULL)
    {
        // we have /proc, likely Linux
        FILE *version = fopen("/proc/version", "r");
        if (version)
        {
            char buffer[256] = {0};
            if (fgets(buffer, sizeof(buffer), version))
            {
                fclose(version);
                if (strstr(buffer, "Linux"))
                {
                    return OS_LINUX;
                }
            }
            fclose(version);
        }
    }

    // check for macOS/iOS
    if (fopen("/System/Library/CoreServices/SystemVersion.plist", "r") != NULL)
    {
        return OS_MACOS;
    }

    return OS_UNKNOWN;
#endif
}

Endian detect_host_endianness()
{
    union
    {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1 ? ENDIAN_BIG : ENDIAN_LITTLE;
}

void detect_host_cpu_features(Features *features)
{
    memset(features, 0, sizeof(Features));

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#if defined(__MMX__)
    features->x86.mmx = true;
#endif
#if defined(__SSE__)
    features->x86.sse = true;
#endif
#if defined(__SSE2__)
    features->x86.sse2 = true;
#endif
#if defined(__SSE3__)
    features->x86.sse3 = true;
#endif
#if defined(__SSSE3__)
    features->x86.ssse3 = true;
#endif
#if defined(__SSE4_1__)
    features->x86.sse4_1 = true;
#endif
#if defined(__SSE4_2__)
    features->x86.sse4_2 = true;
#endif
#if defined(__POPCNT__)
    features->x86.popcnt = true;
#endif
#if defined(__AES__)
    features->x86.aes = true;
#endif
#if defined(__AVX__)
    features->x86.avx = true;
#endif
#if defined(__AVX2__)
    features->x86.avx2 = true;
#endif
#if defined(__BMI__)
    features->x86.bmi1 = true;
#endif
#if defined(__BMI2__)
    features->x86.bmi2 = true;
#endif

    // Fall back to CPUID for features not detected at compile time
    unsigned int regs[4];

#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("cpuid" : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3]) : "a"(1));

    // Only set features that weren't already set by compile-time macros
    if (!features->x86.mmx)
        features->x86.mmx = (regs[3] & (1 << 23)) != 0;
    if (!features->x86.sse)
        features->x86.sse = (regs[3] & (1 << 25)) != 0;
    if (!features->x86.sse2)
        features->x86.sse2 = (regs[3] & (1 << 26)) != 0;
    if (!features->x86.sse3)
        features->x86.sse3 = (regs[2] & (1 << 0)) != 0;
    if (!features->x86.ssse3)
        features->x86.ssse3 = (regs[2] & (1 << 9)) != 0;
    if (!features->x86.sse4_1)
        features->x86.sse4_1 = (regs[2] & (1 << 19)) != 0;
    if (!features->x86.sse4_2)
        features->x86.sse4_2 = (regs[2] & (1 << 20)) != 0;
    if (!features->x86.popcnt)
        features->x86.popcnt = (regs[2] & (1 << 23)) != 0;
    if (!features->x86.aes)
        features->x86.aes = (regs[2] & (1 << 25)) != 0;
    if (!features->x86.avx)
        features->x86.avx = (regs[2] & (1 << 28)) != 0;

    // Check extended features
    __asm__ __volatile__("cpuid" : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3]) : "a"(7), "c"(0));
    if (!features->x86.avx2)
        features->x86.avx2 = (regs[1] & (1 << 5)) != 0;
    if (!features->x86.bmi1)
        features->x86.bmi1 = (regs[1] & (1 << 3)) != 0;
    if (!features->x86.bmi2)
        features->x86.bmi2 = (regs[1] & (1 << 8)) != 0;
#endif

#elif defined(__arm__) || defined(__aarch64__)
    // ARM feature detection

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    features->arm.neon = true;
#endif
#if defined(__ARM_FEATURE_CRC32)
    features->arm.crc = true;
#endif
#if defined(__ARM_FEATURE_CRYPTO)
    features->arm.crypto = true;
    features->arm.aes = true;
    features->arm.sha1 = true;
    features->arm.sha2 = true;
#endif
#if defined(__ARM_FEATURE_SVE)
    features->arm.sve = true;
#endif

#endif
}

Format detect_host_format()
{
    OS host_os = detect_host_os();

    // map OS to its default object format
    switch (host_os)
    {
    case OS_WINDOWS:
        return FORMAT_PE;
    case OS_MACOS:
        return FORMAT_MACHO;
    case OS_LINUX:
        return FORMAT_ELF;
    default:
        return FORMAT_UNKNOWN;
    }
}

CallingConvention detect_host_calling_convention()
{
#if defined(_WIN64)
    return CALL_WIN64;
#elif defined(__x86_64__) && !defined(_WIN32)
    return CALL_SYSV;
#else
    Architecture arch = detect_host_architecture();
    OS os = detect_host_os();

    if (arch == ARCH_X86_64)
    {
        if (os == OS_WINDOWS)
        {
            return CALL_WIN64;
        }
        else
        {
            return CALL_SYSV;
        }
    }

    return CALL_UNKNOWN;
#endif
}
