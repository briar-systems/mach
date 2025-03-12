#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "target.h"

bool valid_platform(Platform platform)
{
    return platform < PLATFORM_UNKNOWN;
}

bool valid_architecture(Architecture arch)
{
    return arch < ARCH_UNKNOWN;
}

bool valid_endian(Endian endian)
{
    return endian < ENDIAN_UNKNOWN;
}

bool valid_target(Target target)
{
    return valid_platform(target.platform) && valid_architecture(target.architecture);
}

Platform platform_from_string(char *str)
{
    for (int i = 0; i < PLATFORM_UNKNOWN; i++)
    {
        if (strcmp(str, platform_names[i].name) == 0)
        {
            return platform_names[i].platform;
        }
    }

    return PLATFORM_UNKNOWN;
}

Architecture architecture_from_string(char *str)
{
    for (int i = 0; i < ARCH_UNKNOWN; i++)
    {
        if (strcmp(str, architecture_names[i].name) == 0)
        {
            return architecture_names[i].arch;
        }
    }

    return ARCH_UNKNOWN;
}

Endian endian_from_string(char *str)
{
    for (int i = 0; i < ENDIAN_UNKNOWN; i++)
    {
        if (strcmp(str, endian_names[i].name) == 0)
        {
            return endian_names[i].endian;
        }
    }

    return ENDIAN_UNKNOWN;
}

Target target_from_string(char *str)
{
    if (str == NULL)
    {
        return (Target){PLATFORM_UNKNOWN, ARCH_UNKNOWN};
    }

    if (strcmp(str, "current") == 0)
    {
        return target_current();
    }

    char *platform_str = strtok(str, &target_delimiter);
    char *arch_str = strtok(NULL, &target_delimiter);

    Platform platform = platform_from_string(platform_str);
    Architecture arch = architecture_from_string(arch_str);

    return (Target){platform, arch};
}

char *platform_to_string(Platform platform)
{
    for (int i = 0; i < PLATFORM_UNKNOWN; i++)
    {
        if (platform == platform_names[i].platform)
        {
            return platform_names[i].name;
        }
    }

    return "unknown";
}

char *architecture_to_string(Architecture arch)
{
    for (int i = 0; i < ARCH_UNKNOWN; i++)
    {
        if (arch == architecture_names[i].arch)
        {
            return architecture_names[i].name;
        }
    }

    return "unknown";
}

char *endian_to_string(Endian endian)
{
    for (int i = 0; i < ENDIAN_UNKNOWN; i++)
    {
        if (endian == endian_names[i].endian)
        {
            return endian_names[i].name;
        }
    }

    return "unknown";
}

char *target_to_string(Target target)
{
    char *platform_str = platform_to_string(target.platform);
    char *arch_str = architecture_to_string(target.architecture);

    char *str = malloc(strlen(platform_str) + strlen(arch_str) + 2);
    sprintf(str, "%s/%s", platform_str, arch_str);

    return str;
}

TargetInfo target_info(Target target)
{
    TargetInfo info = {0};

    switch (target.platform)
    {
    case PLATFORM_WINDOWS:
        info.endian = ENDIAN_LITTLE;
        break;
    case PLATFORM_LINUX:
        info.endian = ENDIAN_LITTLE;
        break;
    case PLATFORM_MACOS:
        info.endian = ENDIAN_LITTLE;
        break;
    default:
        info.endian = ENDIAN_UNKNOWN;
        break;
    }

    switch (target.architecture)
    {
    case ARCH_AMD_X86:
        info.size = 4;
        info.alignment = 4;
        info.register_size = 4;
        break;
    case ARCH_AMD_X64:
        info.size = 8;
        info.alignment = 8;
        info.register_size = 8;
        break;
    case ARCH_ARM:
        info.size = 4;
        info.alignment = 4;
        info.register_size = 4;
        break;
    case ARCH_ARM_64:
        info.size = 8;
        info.alignment = 8;
        info.register_size = 8;
        break;
    default:
        info.size = 0;
        info.alignment = 0;
        info.register_size = 0;
        break;
    }

    return info;
}

Platform platform_current()
{
#if defined(_WIN32) || defined(_WIN64)
    char *platform_str = "windows";
#elif defined(__linux__)
    char *platform_str = "linux";
#elif defined(__APPLE__) && defined(__MACH__)
    char *platform_str = "macos";
#else
    char *platform_str = "unknown";
#endif

    return platform_from_string(platform_str);
}

Architecture architecture_current()
{
    char *arch_str = NULL;

#if defined(_M_IX86) || defined(__i386__)
    arch_str = "x86";
#elif defined(_M_X64) || defined(__x86_64__)
    arch_str = "x64";
#elif defined(__arm__)
    arch_str = "arm";
#elif defined(__aarch64__)
    arch_str = "arm64";
#endif

    if (arch_str)
    {
        return architecture_from_string(arch_str);
    }

    return ARCH_UNKNOWN;
}

Target target_current()
{
    Target target = {
        .platform = platform_current(),
        .architecture = architecture_current(),
    };

    return target;
}
