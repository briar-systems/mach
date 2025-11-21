#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// target modes
typedef enum
{
    TARGET_MODE_EXECUTABLE,
    TARGET_MODE_LIBRARY,
    TARGET_MODE_SHARED
} TargetModeKind;

typedef struct TargetMode
{
    TargetModeKind kind;
    const char    *value;
} TargetMode;

static const TargetMode TARGET_MODES[] = {{TARGET_MODE_EXECUTABLE, "executable"}, {TARGET_MODE_LIBRARY, "library"}, {TARGET_MODE_SHARED, "shared"}};

// target platforms
typedef enum
{
    TARGET_PLATFORM_LINUX,
} TargetPlatformKind;

typedef struct TargetPlatform
{
    TargetPlatformKind kind;
    const char        *value;
} TargetPlatform;

static const TargetPlatform TARGET_PLATFORMS[] = {
    {TARGET_PLATFORM_LINUX, "linux"},
};

// target architectures
typedef enum
{
    TARGET_ARCH_X86_64,
} TargetArchKind;

typedef struct TargetArch
{
    TargetArchKind kind;
    const char    *value;
} TargetArch;

static const TargetArch TARGET_ARCHS[] = {
    {TARGET_ARCH_X86_64, "x86_64"},
};

// dependency types
typedef enum
{
    DEP_TYPE_REMOTE,
    DEP_TYPE_LOCAL
} DepTypeKind;

typedef struct DepType
{
    DepTypeKind kind;
    const char *value;
} DepType;

static const DepType DEP_TYPES[] = {
    {DEP_TYPE_REMOTE, "remote"},
    {DEP_TYPE_LOCAL, "local"},
};

// dependency version kinds
typedef enum
{
    DEP_VERSION_KIND_BRANCH,
    DEP_VERSION_KIND_SEMVER,
    DEP_VERSION_KIND_COMMIT
} DepVersionKind;

typedef struct DepVersion
{
    DepVersionKind kind;
    const char    *value;
} DepVersion;

// target-specific configuration
typedef struct TargetConfig
{
    char           *name;       // target name
    TargetPlatform *platform;   // target platform/os
    TargetArch     *arch;       // target architecture
    TargetMode     *mode;       // build mode: executable|library
    char           *entrypoint; // main source file (relative to src_dir)
    char           *artifacts;  // artifacts directory (relative to out_dir)
    char           *binary;     // output binary path (relative to out_dir)
} TargetConfig;

// explicit dependency specification (parsed from [deps] tables)
typedef struct DepSpec
{
    char       *name;    // dependency name
    DepType    *type;    // dependency type: remote|local
    char       *path;    // remote URL or local filesystem path (used by dep tooling)
    DepVersion *version; // version specifier (for remote deps): branch/semver/commit
} DepSpec;

// project configuration
typedef struct ProjectConfig
{
    char *id;      // project id (used for module prefix and soft uniqueness)
    char *name;    // project name (canonical, human-readable)
    char *version; // project version
    char *dir_src; // source files directory
    char *dir_out; // output files directory
    char *dir_dep; // dependencies directory
    char *target;  // target name (or "native" to auto-detect)

    TargetConfig **targets;
    int            target_count;

    DepSpec **deps;
    int       dep_count;
} ProjectConfig;

// target management
void target_config_init(TargetConfig *target);
void target_config_dnit(TargetConfig *target);

// dependency management
void dep_spec_init(DepSpec *dep);
void dep_spec_dnit(DepSpec *dep);

// configuration lifecycle
void config_init(ProjectConfig *config);
void config_dnit(ProjectConfig *config);

// configuration file management
ProjectConfig *config_load(const char *config_path);
bool           config_save(ProjectConfig *config, const char *config_path);

bool          config_add_target(ProjectConfig *config, const char *name, const char *target_triple);
TargetConfig *config_get_target(ProjectConfig *config, const char *name);
bool          config_add_dependency(ProjectConfig *config, DepSpec *dep);
DepSpec      *config_get_dependency(ProjectConfig *config, const char *name);
bool          config_del_dependency(ProjectConfig *config, const char *name);

#endif
