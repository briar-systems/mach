#ifndef TARGET_H
#define TARGET_H

#include <stdbool.h>

typedef enum Platform
{
    PLATFORM_WINDOWS = 0,
    PLATFORM_LINUX   = 1,
    PLATFORM_MACOS   = 2,

    PLATFORM_UNKNOWN
} Platform;

typedef enum Architecture
{
    ARCH_AMD_X86 = 0,
    ARCH_AMD_X64 = 1,
    ARCH_ARM     = 2,
    ARCH_ARM_64  = 3,

    ARCH_UNKNOWN
} Architecture;

typedef enum Endian
{
    ENDIAN_LITTLE = 0,
    ENDIAN_BIG    = 1,

    ENDIAN_UNKNOWN
} Endian;

typedef struct Target
{
    Platform platform;
    Architecture architecture;
} Target;

typedef struct TargetInfo
{
    Endian endian;
    int size;
    int alignment;
    int register_size;
} TargetInfo;

typedef struct platform_name_map
{
    Platform platform;
    char *name;
} platform_name_map;

typedef struct architecture_name_map
{
    Architecture arch;
    char *name;
} architecture_name_map;

typedef struct endian_name_map
{
    Endian endian;
    char *name;
} endian_name_map;

static const platform_name_map platform_names[] = {
    {PLATFORM_WINDOWS, "windows"},
    {PLATFORM_LINUX, "linux"},
    {PLATFORM_MACOS, "macos"},
};

static const architecture_name_map architecture_names[] = {
    {ARCH_AMD_X86, "x86"},
    {ARCH_AMD_X64, "x64"},
    {ARCH_ARM, "arm"},
    {ARCH_ARM_64, "arm64"},
};

static const endian_name_map endian_names[] = {
    {ENDIAN_LITTLE, "little"},
    {ENDIAN_BIG, "big"},
};

static const char target_delimiter = '/';

bool valid_platform(Platform platform);
bool valid_architecture(Architecture arch);
bool valid_endian(Endian endian);
bool valid_target(Target target);

Platform platform_from_string(char *str);
Architecture architecture_from_string(char *str);
Endian endian_from_string(char *str);
Target target_from_string(char *str);

char *platform_to_string(Platform platform);
char *architecture_to_string(Architecture arch);
char *endian_to_string(Endian endian);
char *target_to_string(Target target);

Platform platform_current();
Architecture architecture_current();
Target target_current();

TargetInfo target_info(Target target);

#endif // TARGET_H
