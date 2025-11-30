#ifndef CONFIG_H
#define CONFIG_H

#include "compiler/mir/target.h"

#include <stdbool.h>

// target modes
typedef enum
{
    TARGET_MODE_EXECUTABLE,
    TARGET_MODE_LIBRARY,
    TARGET_MODE_SHARED
} ConfigTargetModeKind;

typedef struct ConfigTargetMode
{
    ConfigTargetModeKind kind;
    const char          *value;
} ConfigTargetMode;

static const ConfigTargetMode TARGET_MODES[] = {{TARGET_MODE_EXECUTABLE, "executable"}, {TARGET_MODE_LIBRARY, "library"}, {TARGET_MODE_SHARED, "shared"}};

// dependency types
typedef enum
{
    DEP_TYPE_REMOTE,
    DEP_TYPE_LOCAL
} ConfigDepTypeKind;

typedef struct ConfigDepType
{
    ConfigDepTypeKind kind;
    const char       *value;
} ConfigDepType;

static const ConfigDepType DEP_TYPES[] = {
    {DEP_TYPE_REMOTE, "remote"},
    {DEP_TYPE_LOCAL, "local"},
};

// dependency version kinds
typedef enum
{
    DEP_VERSION_KIND_BRANCH,
    DEP_VERSION_KIND_SEMVER,
    DEP_VERSION_KIND_COMMIT
} ConfigDepVersionKind;

typedef struct ConfigDepVersion
{
    ConfigDepVersionKind kind;
    const char          *value;
} ConfigDepVersion;

// target-specific configuration
typedef struct ConfigTarget
{
    char             *name;       // target name
    MIRTargetOS       os;         // target operating system
    MIRTargetISA      isa;        // target ISA
    MIRTargetABI      abi;        // target ABI
    ConfigTargetMode *mode;       // build mode: executable|library
    char             *entrypoint; // main source file (relative to src_dir)
    char             *artifacts;  // artifacts directory (relative to out_dir)
    char             *binary;     // output binary path (relative to out_dir)
} ConfigTarget;

// explicit dependency specification (parsed from [deps] tables)
typedef struct Config Config; // forward declaration
typedef struct ConfigDep
{
    char             *name;    // dependency name (key in [deps] table)
    ConfigDepType    *type;    // dependency type: remote|local
    char             *path;    // remote URL or local filesystem path (used by dep tooling)
    ConfigDepVersion *version; // version specifier (for remote deps): branch/semver/commit
    Config           *config;  // loaded dependency config (contains project.id and dir_src)
} ConfigDep;

// project configuration
typedef struct Config
{
    char *id;      // project id (used for module prefix and soft uniqueness)
    char *name;    // project name (canonical, human-readable)
    char *version; // project version
    char *dir_src; // source files directory
    char *dir_out; // output files directory
    char *dir_dep; // dependencies directory
    char *target;  // target name (or "native" to auto-detect)

    ConfigTarget **targets;
    int            target_count;

    ConfigDep **deps;
    int         dep_count;
} Config;

// target management
void target_config_init(ConfigTarget *target);
void target_config_dnit(ConfigTarget *target);

// dependency management
void dep_spec_init(ConfigDep *dep);
void dep_spec_dnit(ConfigDep *dep);

// configuration lifecycle
void config_init(Config *config);
void config_dnit(Config *config);

// configuration file management
Config *config_load(const char *config_path);
bool    config_save(Config *config, const char *config_path);

bool          config_add_target(Config *config, ConfigTarget *target);
ConfigTarget *config_get_target(Config *config, const char *name);
bool          config_add_dependency(Config *config, ConfigDep *dep);
ConfigDep    *config_get_dependency(Config *config, const char *name);
bool          config_del_dependency(Config *config, const char *name);

#endif
