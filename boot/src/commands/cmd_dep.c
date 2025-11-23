#include "commands/cmd_dep.h"
#include "config.h"
#include "filesystem.h"
#include "git.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __WIN32
static const char *MACH_TOML_CONFIG_PATH = ".\\mach.toml";
#else
static const char *MACH_TOML_CONFIG_PATH = "./mach.toml";
#endif

void cmd_dep_help(FILE *stream)
{
    fprintf(stream, "usage: mach dep <command> [options]\n");
    fprintf(stream, "\n");
    fprintf(stream, "commands:\n");
    fprintf(stream, "  list                 list dependencies in the current project\n");
    fprintf(stream, "  info <name>          show information about a dependency\n");
    fprintf(stream, "  tidy                 perform a submodule maintenance pass\n");
    fprintf(stream, "  add [--local] <path> [name] [--version <version>]\n");
    fprintf(stream, "                       register a dependency (assumes remote by default)\n");
    fprintf(stream, "  del  <name>          remove a dependency entry\n");
    fprintf(stream, "  pull [name]          refresh vendored dependencies\n");
    fprintf(stream, "\n");
    fprintf(stream, "version format examples:\n");
    fprintf(stream, "  branch/main          track a specific branch\n");
    fprintf(stream, "  commit/hash          specific commit\n");
    fprintf(stream, "  ^1.2.3               semver caret (>=1.2.3 <2.0.0)\n");
    fprintf(stream, "  ~1.2.3               semver tilde (>=1.2.3 <1.3.0)\n");
    fprintf(stream, "  1.2.3                exact semver tag\n");
}

// copy directory contents recursively (for local dependencies)
static bool copy_directory_recursive(const char *src, const char *dest)
{
    if (!src || !dest)
    {
        return false;
    }

    // ensure destination directory exists
    if (!ensure_dir_recursive(dest))
    {
        return false;
    }

    // use platform-specific copy command
#ifdef _WIN32
    size_t cmd_len = strlen("xcopy /E /I /Y \"\" \"\"") + strlen(src) + strlen(dest) + 1;
    char  *cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }
    snprintf(cmd, cmd_len, "xcopy /E /I /Y \"%s\" \"%s\"", src, dest);
#else
    size_t cmd_len = strlen("cp -r \"\"/* \"\"") + strlen(src) + strlen(dest) + 1;
    char  *cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }
    snprintf(cmd, cmd_len, "cp -r \"%s\"/* \"%s\"", src, dest);
#endif

    int result = system(cmd);
    free(cmd);

    return result == 0;
}

// pull a specific dependency
// this function can do a few differnt things based on the dependency type:
// local:
// - always copies files from the local path into the dependency destination
// remote:
// - ensures the git submodule exists and is initialized
// - if it exists, does a git fetch and checkout to the specified version
static int pull_dependency(Config *config, ConfigDep *dep)
{
    if (!config || !dep || !dep->name || !dep->path)
    {
        fprintf(stderr, "error: invalid dependency specification\n");
        return 1;
    }

    const char *dep_dir = config->dir_dep ? config->dir_dep : "dep";

    // construct destination path: <dir_dep>/<name>
    char *dep_dest = path_join(dep_dir, dep->name);
    if (!dep_dest)
    {
        fprintf(stderr, "error: failed to construct dependency path\n");
        return 1;
    }

    // handle based on dependency type
    if (dep->type && dep->type->kind == DEP_TYPE_LOCAL)
    {
        // local dependency: copy files from source to destination
        printf("  copying local dependency from '%s'...\n", dep->path);
        
        if (!copy_directory_recursive(dep->path, dep_dest))
        {
            fprintf(stderr, "error: failed to copy local dependency\n");
            free(dep_dest);
            return 1;
        }

        printf("  local dependency copied successfully\n");
        free(dep_dest);
        return 0;
    }
    else
    {
        // remote dependency: handle as git submodule
        printf("  checking remote dependency at '%s'...\n", dep->path);

        bool is_initialized = git_is_repo(dep_dest);

        if (!is_initialized)
        {
            // initialize new submodule
            printf("  initializing git submodule...\n");
            if (!git_submodule_init(dep_dest, dep->path))
            {
                fprintf(stderr, "error: failed to initialize git submodule\n");
                free(dep_dest);
                return 1;
            }
        }
        else
        {
            // update existing submodule
            printf("  updating existing git submodule...\n");
            if (!git_submodule_fetch(dep_dest))
            {
                fprintf(stderr, "warning: failed to fetch latest changes\n");
            }
        }

        // checkout specific version if specified
        if (dep->version && dep->version->value)
        {
            printf("  checking out version '%s'...\n", dep->version->value);
            if (!git_checkout_version(dep_dest, dep->version->value))
            {
                fprintf(stderr, "error: failed to checkout version '%s'\n", dep->version->value);
                free(dep_dest);
                return 1;
            }
        }

        printf("  remote dependency synchronized successfully\n");
        free(dep_dest);
        return 0;
    }
}

// this function does a few things:
// - ensures the .gitmodules and .git/config entries are correct
// - ensures the submodule is initialized and updated
// - removes any orphaned submodule data if necessary
static int submodules_maintain(Config *config)
{
    if (!config)
    {
        return 1;
    }

    // check if we're in a git repository
    if (!git_is_repo("."))
    {
        // not a git repo, skip submodule maintenance
        return 0;
    }

    printf("performing submodule maintenance...\n");

    const char *dep_dir = config->dir_dep ? config->dir_dep : "dep";

    // get list of all submodules in dependency directory
    char **dep_entries = list_files(dep_dir);
    if (dep_entries)
    {
        for (int i = 0; dep_entries[i]; i++)
        {
            char *dep_basename = path_basename(dep_entries[i]);
            if (!dep_basename)
            {
                continue;
            }

            // check if this submodule is still in config
            bool found = false;
            for (int j = 0; j < config->dep_count; j++)
            {
                ConfigDep *dep = config->deps[j];
                if (dep && dep->name && strcmp(dep->name, dep_basename) == 0)
                {
                    if (!dep->type || dep->type->kind == DEP_TYPE_REMOTE)
                    {
                        found = true;
                        break;
                    }
                }
            }

            // if not found and it's a git repo, remove it as an orphaned submodule
            if (!found && git_is_repo(dep_entries[i]))
            {
                printf("  removing orphaned submodule '%s'...\n", dep_basename);
                if (!git_submodule_remove(dep_entries[i]))
                {
                    fprintf(stderr, "warning: failed to remove orphaned submodule '%s'\n", dep_basename);
                }
            }

            free(dep_basename);
        }
        free_string_array(dep_entries);
    }

    // sync submodule configuration
    int result = git_exec("git submodule sync");
    if (result != 0)
    {
        fprintf(stderr, "warning: failed to sync submodule configuration\n");
    }

    // for each dependency, ensure it's properly set up
    for (int i = 0; i < config->dep_count; i++)
    {
        ConfigDep *dep = config->deps[i];
        if (!dep || !dep->name)
        {
            continue;
        }

        // only handle remote dependencies as submodules
        if (!dep->type || dep->type->kind != DEP_TYPE_REMOTE)
        {
            continue;
        }

        char *dep_path = path_join(dep_dir, dep->name);
        if (!dep_path)
        {
            continue;
        }

        // ensure submodule is initialized
        if (git_is_repo(dep_path))
        {
            git_submodule_update(dep_path);
        }

        free(dep_path);
    }

    printf("submodule maintenance complete\n");
    return 0;
}

// list dependencies in the current project
static int handle_dep_list(int argc, char **argv)
{
    (void)argc; // unused
    (void)argv; // unused

    Config *config = config_load(MACH_TOML_CONFIG_PATH);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml from current directory\n");
        return 1;
    }

    if (config->dep_count == 0)
    {
        printf("no dependencies declared\n");
        config_dnit(config);
        return 0;
    }

    for (int i = 0; i < config->dep_count; i++)
    {
        ConfigDep *dep = config->deps[i];
        if (!dep)
        {
            continue;
        }

        printf("%s\n", dep->name ? dep->name : "unknown");
    }

    config_dnit(config);
    return 0;
}

// show detailed information about a specific dependency
static int handle_dep_info(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "error: dep info requires a dependency name\n");
        return 1;
    }

    const char *dep_name = argv[3];

    Config *config = config_load(MACH_TOML_CONFIG_PATH);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml from current directory\n");
        return 1;
    }

    ConfigDep *dep = config_get_dependency(config, dep_name);
    if (!dep)
    {
        fprintf(stderr, "error: dependency '%s' not found in mach.toml\n", dep_name);
        config_dnit(config);
        return 1;
    }

    char       *name    = dep->name ? dep->name : "unknown";
    const char *type    = dep->type ? dep->type->value : "unknown";
    const char *path    = dep->path ? dep->path : "unknown";
    const char *version = dep->version ? dep->version->value : NULL;

    printf("name:    %s\n", name);
    printf("type:    %s\n", type);
    printf("path:    %s\n", path);
    if (version != NULL)
    {
        printf("version: %s\n", version);
    }

    config_dnit(config);
    return 0;
}

// perform submodule maintenance
static int handle_dep_tidy(int argc, char **argv)
{
    (void)argc; // unused
    (void)argv; // unused

    Config *config = config_load(MACH_TOML_CONFIG_PATH);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml from current directory\n");
        return 1;
    }

    int result = submodules_maintain(config);
    if (result != 0)
    {
        fprintf(stderr, "error: failed to maintain submodules\n");
    }

    config_dnit(config);
    return result;
}

// delete a dependency from the current project
static int handle_dep_del(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "error: dep del requires a dependency name\n");
        return 1;
    }

    const char *dep_name = argv[3];

    Config *config = config_load(MACH_TOML_CONFIG_PATH);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml from current directory\n");
        return 1;
    }

    bool success = config_del_dependency(config, dep_name);
    if (!success)
    {
        fprintf(stderr, "error: dependency '%s' not found in mach.toml\n", dep_name);
        config_dnit(config);
        return 1;
    }

    success = config_save(config, MACH_TOML_CONFIG_PATH);
    if (!success)
    {
        fprintf(stderr, "error: failed to save updated mach.toml\n");
        config_dnit(config);
        return 1;
    }

    printf("dependency '%s' removed successfully\n", dep_name);

    // remove the submodule if it exists
    if (git_is_repo("."))
    {
        const char *dep_dir = config->dir_dep ? config->dir_dep : "dep";
        char *dep_path = path_join(dep_dir, dep_name);
        if (dep_path && git_is_repo(dep_path))
        {
            printf("removing submodule '%s'...\n", dep_name);
            if (!git_submodule_remove(dep_path))
            {
                fprintf(stderr, "warning: failed to remove submodule '%s'\n", dep_name);
            }
        }
        free(dep_path);
    }

    config_dnit(config);
    return 0;
}

// add a dependency to the current project
static int handle_dep_add(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "error: dep add requires at least a path/url argument\n");
        cmd_dep_help(stderr);
        return 1;
    }

    int         is_local         = 0;
    const char *path_or_url      = NULL;
    const char *dep_name         = NULL;
    const char *version_override = NULL;

    // parse arguments
    int arg_idx = 3;
    while (arg_idx < argc)
    {
        if (strcmp(argv[arg_idx], "--local") == 0)
        {
            is_local = 1;
            arg_idx++;
        }
        else if (strcmp(argv[arg_idx], "--version") == 0)
        {
            if (arg_idx + 1 >= argc)
            {
                fprintf(stderr, "error: --version requires a version argument\n");
                return 1;
            }
            version_override = argv[arg_idx + 1];
            arg_idx += 2;
        }
        else
        {
            if (!path_or_url)
            {
                path_or_url = argv[arg_idx];
            }
            else if (!dep_name)
            {
                dep_name = argv[arg_idx];
            }
            else
            {
                fprintf(stderr, "error: unexpected argument '%s'\n", argv[arg_idx]);
                return 1;
            }
            arg_idx++;
        }
    }
    if (!path_or_url)
    {
        fprintf(stderr, "error: dep add requires a path/url argument\n");
        return 1;
    }

    Config *config = config_load(MACH_TOML_CONFIG_PATH);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml from current directory\n");
        return 1;
    }

    // allocate and initialize dependency
    ConfigDep *dep = malloc(sizeof(ConfigDep));
    if (!dep)
    {
        fprintf(stderr, "error: failed to allocate memory for dependency\n");
        config_dnit(config);
        return 1;
    }
    dep_spec_init(dep);
    
    dep->name = dep_name ? strdup(dep_name) : NULL;
    dep->path = strdup(path_or_url);
    
    // allocate and initialize type
    dep->type = malloc(sizeof(ConfigDepType));
    if (!dep->type)
    {
        fprintf(stderr, "error: failed to allocate memory for dependency type\n");
        free(dep->name);
        free(dep->path);
        free(dep);
        config_dnit(config);
        return 1;
    }
    dep->type->kind  = is_local ? DEP_TYPE_LOCAL : DEP_TYPE_REMOTE;
    dep->type->value = is_local ? "local" : "remote";
    
    // handle version override if provided
    if (version_override)
    {
        dep->version = malloc(sizeof(ConfigDepVersion));
        if (dep->version)
        {
            dep->version->value = strdup(version_override);
            // determine version kind based on format
            if (strncmp(version_override, "branch/", 7) == 0)
            {
                dep->version->kind = DEP_VERSION_KIND_BRANCH;
            }
            else if (strncmp(version_override, "commit/", 7) == 0)
            {
                dep->version->kind = DEP_VERSION_KIND_COMMIT;
            }
            else
            {
                dep->version->kind = DEP_VERSION_KIND_SEMVER;
            }
        }
    }

    bool success = config_add_dependency(config, dep);
    if (!success)
    {
        fprintf(stderr, "error: failed to add dependency '%s'\n", dep_name ? dep_name : path_or_url);
        config_dnit(config);
        return 1;
    }

    success = config_save(config, MACH_TOML_CONFIG_PATH);
    if (!success)
    {
        fprintf(stderr, "error: failed to save updated mach.toml\n");
        config_dnit(config);
        return 1;
    }

    printf("dependency '%s' added successfully\n", dep_name ? dep_name : path_or_url);

    // initialize the submodule if it's a remote dependency
    if (git_is_repo(".") && dep->type && dep->type->kind == DEP_TYPE_REMOTE)
    {
        const char *dep_dir = config->dir_dep ? config->dir_dep : "dep";
        char *dep_path = path_join(dep_dir, dep->name);
        if (dep_path)
        {
            if (!git_is_repo(dep_path))
            {
                printf("initializing submodule '%s'...\n", dep->name);
                if (!git_submodule_init(dep_path, dep->path))
                {
                    fprintf(stderr, "warning: failed to initialize submodule '%s'\n", dep->name);
                }
            }
            free(dep_path);
        }
    }

    config_dnit(config);
    return 0;
}

static int handle_dep_pull(int argc, char **argv)
{
    const char *specific_dep = NULL;
    if (argc >= 4)
    {
        specific_dep = argv[3];
    }

    Config *config = config_load(MACH_TOML_CONFIG_PATH);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml from current directory\n");
        return 1;
    }

    if (config->dep_count == 0)
    {
        printf("project has no dependencies\n");
        config_dnit(config);
        return 0;
    }

    for (int i = 0; i < config->dep_count; i++)
    {
        ConfigDep *dep = config->deps[i];
        if (!dep)
        {
            continue;
        }

        if (specific_dep && dep->name && strcmp(dep->name, specific_dep) != 0)
        {
            continue;
        }

        printf("pulling dependency '%s'...\n", dep->name ? dep->name : "unknown");
        if (!pull_dependency(config, dep))
        {
            fprintf(stderr, "error: failed to pull dependency '%s'\n", dep->name ? dep->name : "unknown");
            config_dnit(config);
            return 1;
        }
        printf("dependency '%s' pulled successfully\n", dep->name ? dep->name : "unknown");
    }

    config_dnit(config);
    return 0;
}

int cmd_dep_handle(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "error: dep command requires a subcommand\n");
        cmd_dep_help(stderr);
        return 1;
    }

    const char *subcommand = argv[2];

    if (strcmp(subcommand, "list") == 0)
    {
        return handle_dep_list(argc, argv);
    }
    else if (strcmp(subcommand, "info") == 0)
    {
        return handle_dep_info(argc, argv);
    }
    else if (strcmp(subcommand, "tidy") == 0)
    {
        return handle_dep_tidy(argc, argv);
    }
    else if (strcmp(subcommand, "add") == 0)
    {
        return handle_dep_add(argc, argv);
    }
    else if (strcmp(subcommand, "del") == 0)
    {
        return handle_dep_del(argc, argv);
    }
    else if (strcmp(subcommand, "pull") == 0)
    {
        return handle_dep_pull(argc, argv);
    }
    else
    {
        fprintf(stderr, "error: unknown dep subcommand '%s'\n", subcommand);
        cmd_dep_help(stderr);
        return 1;
    }
}
