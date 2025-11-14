#include "commands.h"
#include "config.h"
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#else
#include <linux/limits.h>
#endif

// forward declarations for subcommands
static int  cmd_dep_list(int argc, char **argv);
static int  cmd_dep_info(int argc, char **argv);
static int  cmd_dep_add(int argc, char **argv);
static int  cmd_dep_del(int argc, char **argv);
static int  cmd_dep_pull(int argc, char **argv);
static void cmd_dep_help(void);

// helper functions
static int   run_git_command(const char *command);
static int   vendor_dir_exists(const char *path);
static char *find_project_root(const char *start_path);
static int   config_add_dep_and_save(ProjectConfig *config, const char *config_path, DepSpec *dep);
static int   config_remove_dep_and_save(ProjectConfig *config, const char *config_path, const char *dep_name);
static int   parse_semver_tag(const char *tag, int *major, int *minor, int *patch);
static int   semver_matches(const char *version_constraint, int major, int minor, int patch);

int mach_cmd_dep(int argc, char **argv)
{
    if (argc < 3)
    {
        cmd_dep_help();
        return 1;
    }

    const char *subcommand = argv[2];

    if (strcmp(subcommand, "list") == 0)
    {
        return cmd_dep_list(argc, argv);
    }
    else if (strcmp(subcommand, "info") == 0)
    {
        return cmd_dep_info(argc, argv);
    }
    else if (strcmp(subcommand, "add") == 0)
    {
        return cmd_dep_add(argc, argv);
    }
    else if (strcmp(subcommand, "del") == 0)
    {
        return cmd_dep_del(argc, argv);
    }
    else if (strcmp(subcommand, "pull") == 0)
    {
        return cmd_dep_pull(argc, argv);
    }
    else if (strcmp(subcommand, "help") == 0)
    {
        cmd_dep_help();
        return 0;
    }
    else
    {
        fprintf(stderr, "error: unknown dep subcommand '%s'\n", subcommand);
        cmd_dep_help();
        return 1;
    }
}

static void cmd_dep_help(void)
{
    printf("usage: mach dep <command> [options]\n");
    printf("\n");
    printf("commands:\n");
    printf("  list                 list dependencies in the current project\n");
    printf("  info <name>          show information about a dependency\n");
    printf("  add  [--local] <path> [name]\n");
    printf("                       register a dependency (assumes remote by default)\n");
    printf("  del  <name>          remove a dependency entry\n");
    printf("  pull [name]          refresh vendored dependencies\n");
    printf("\n");
    printf("examples:\n");
    printf("  mach dep info std\n");
    printf("  mach dep list\n");
    printf("  mach dep add https://example.com/foo.git\n");
    printf("  mach dep add --local path/to/proj std\n");
    printf("  mach dep del std\n");
    printf("  mach dep pull\n");
}

static int cmd_dep_list(int argc, char **argv)
{
    (void)argc; // unused
    (void)argv; // unused

    char *project_root = find_project_root(".");
    if (!project_root)
    {
        fprintf(stderr, "error: not in a mach project (no mach.toml found)\n");
        return 1;
    }

    ProjectConfig *config = config_load_from_dir(project_root);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml\n");
        free(project_root);
        return 1;
    }

    if (config->dep_count == 0)
    {
        printf("no dependencies declared\n");
        config_dnit(config);
        free(config);
        free(project_root);
        return 0;
    }

    printf("dependencies:\n");
    for (int i = 0; i < config->dep_count; i++)
    {
        DepSpec *dep = config->deps[i];
        if (!dep)
            continue;

        printf("  %s\n", dep->name ? dep->name : "(unnamed)");
        printf("    type: %s\n", dep->type ? dep->type : "unknown");
        printf("    source: %s\n", dep->path ? dep->path : "(none)");
        if (dep->version && strlen(dep->version) > 0)
        {
            printf("    version: %s\n", dep->version);
        }

        char vendor_path[PATH_MAX];
        snprintf(vendor_path, sizeof(vendor_path), "%s/dep/%s", project_root, dep->name ? dep->name : "(unnamed)");
        printf("    vendor: %s\n", vendor_path);
    }

    config_dnit(config);
    free(config);
    free(project_root);
    return 0;
}

static int cmd_dep_info(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "error: dep info requires a dependency name\n");
        return 1;
    }

    const char *dep_name = argv[3];

    char *project_root = find_project_root(".");
    if (!project_root)
    {
        fprintf(stderr, "error: not in a mach project (no mach.toml found)\n");
        return 1;
    }

    ProjectConfig *config = config_load_from_dir(project_root);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml\n");
        free(project_root);
        return 1;
    }

    DepSpec *dep = NULL;
    for (int i = 0; i < config->dep_count; i++)
    {
        if (config->deps[i] && config->deps[i]->name && strcmp(config->deps[i]->name, dep_name) == 0)
        {
            dep = config->deps[i];
            break;
        }
    }

    if (!dep)
    {
        fprintf(stderr, "error: dependency '%s' not found in mach.toml\n", dep_name);
        config_dnit(config);
        free(config);
        free(project_root);
        return 1;
    }

    char vendor_path[PATH_MAX];
    snprintf(vendor_path, sizeof(vendor_path), "%s/dep/%s", project_root, dep->name);
    int exists = vendor_dir_exists(vendor_path);

    printf("dependency '%s'\n", dep->name);
    printf("  type: %s\n", dep->type ? dep->type : "unknown");
    printf("  source: %s\n", dep->path ? dep->path : "(none)");
    if (dep->version && strlen(dep->version) > 0)
    {
        printf("  version: %s\n", dep->version);
    }
    printf("  vendor dir: %s\n", vendor_path);
    printf("  status: %s\n", exists ? "vendored" : "missing");

    config_dnit(config);
    free(config);
    free(project_root);
    return 0;
}

static int cmd_dep_add(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "error: dep add requires at least a path/url argument\n");
        cmd_dep_help();
        return 1;
    }

    int         is_local    = 0;
    const char *path_or_url = NULL;
    const char *dep_name    = NULL;

    // parse arguments
    int arg_idx = 3;
    if (strcmp(argv[arg_idx], "--local") == 0)
    {
        is_local = 1;
        arg_idx++;
        if (argc <= arg_idx)
        {
            fprintf(stderr, "error: --local requires a path argument\n");
            return 1;
        }
    }

    path_or_url = argv[arg_idx];
    arg_idx++;

    if (argc > arg_idx)
    {
        dep_name = argv[arg_idx];
    }
    else
    {
        // extract name from path/url
        const char *last_slash = strrchr(path_or_url, '/');
        if (last_slash)
        {
            dep_name = last_slash + 1;
        }
        else
        {
            dep_name = path_or_url;
        }

        // remove .git extension if present
        char *name_copy = strdup(dep_name);
        char *dot_git   = strstr(name_copy, ".git");
        if (dot_git && *(dot_git + 4) == '\0')
        {
            *dot_git = '\0';
        }
        dep_name = name_copy;
    }

    char *project_root = find_project_root(".");
    if (!project_root)
    {
        fprintf(stderr, "error: not in a mach project (no mach.toml found)\n");
        return 1;
    }

    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s/mach.toml", project_root);

    ProjectConfig *config = config_load_from_dir(project_root);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml\n");
        free(project_root);
        return 1;
    }

    // check if dependency already exists
    for (int i = 0; i < config->dep_count; i++)
    {
        if (config->deps[i] && config->deps[i]->name && strcmp(config->deps[i]->name, dep_name) == 0)
        {
            fprintf(stderr, "error: dependency '%s' already exists\n", dep_name);
            config_dnit(config);
            free(config);
            free(project_root);
            return 1;
        }
    }

    // create new DepSpec
    DepSpec *new_dep = calloc(1, sizeof(DepSpec));
    new_dep->name    = strdup(dep_name);
    new_dep->type    = strdup(is_local ? "local" : "remote");
    new_dep->path    = strdup(path_or_url);
    new_dep->version = NULL;

    // for remote deps, add git submodule
    if (!is_local)
    {
        char vendor_path[PATH_MAX];
        snprintf(vendor_path, sizeof(vendor_path), "dep/%s", dep_name);

        char git_cmd[PATH_MAX * 2];
        snprintf(git_cmd, sizeof(git_cmd), "cd %s && git submodule add %s %s", project_root, path_or_url, vendor_path);

        printf("adding git submodule for '%s'...\n", dep_name);
        if (run_git_command(git_cmd) != 0)
        {
            fprintf(stderr, "error: failed to add git submodule\n");
            free(new_dep->name);
            free(new_dep->type);
            free(new_dep->path);
            free(new_dep);
            config_dnit(config);
            free(config);
            free(project_root);
            return 1;
        }
    }

    // add to config and save
    if (config_add_dep_and_save(config, config_path, new_dep) != 0)
    {
        fprintf(stderr, "error: failed to update mach.toml\n");
        free(new_dep->name);
        free(new_dep->type);
        free(new_dep->path);
        free(new_dep);
        config_dnit(config);
        free(config);
        free(project_root);
        return 1;
    }

    printf("dependency '%s' added successfully\n", dep_name);

    config_dnit(config);
    free(config);
    free(project_root);
    return 0;
}

static int cmd_dep_del(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "error: dep del requires a dependency name\n");
        return 1;
    }

    const char *dep_name = argv[3];

    char *project_root = find_project_root(".");
    if (!project_root)
    {
        fprintf(stderr, "error: not in a mach project (no mach.toml found)\n");
        return 1;
    }

    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s/mach.toml", project_root);

    ProjectConfig *config = config_load_from_dir(project_root);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml\n");
        free(project_root);
        return 1;
    }

    // find dependency
    DepSpec *dep = NULL;
    for (int i = 0; i < config->dep_count; i++)
    {
        if (config->deps[i] && config->deps[i]->name && strcmp(config->deps[i]->name, dep_name) == 0)
        {
            dep = config->deps[i];
            break;
        }
    }

    if (!dep)
    {
        fprintf(stderr, "error: dependency '%s' not found\n", dep_name);
        config_dnit(config);
        free(config);
        free(project_root);
        return 1;
    }

    // for remote deps, remove git submodule
    if (dep->type && strcmp(dep->type, "remote") == 0)
    {
        char vendor_path[PATH_MAX];
        snprintf(vendor_path, sizeof(vendor_path), "dep/%s", dep_name);

        char git_cmd[PATH_MAX * 2];
        snprintf(git_cmd, sizeof(git_cmd), "cd %s && git submodule deinit -f %s && git rm -f %s", project_root, vendor_path, vendor_path);

        printf("removing git submodule for '%s'...\n", dep_name);
        if (run_git_command(git_cmd) != 0)
        {
            fprintf(stderr, "warning: failed to remove git submodule (continuing anyway)\n");
        }
    }

    // remove from config and save
    if (config_remove_dep_and_save(config, config_path, dep_name) != 0)
    {
        fprintf(stderr, "error: failed to update mach.toml\n");
        config_dnit(config);
        free(config);
        free(project_root);
        return 1;
    }

    printf("dependency '%s' removed successfully\n", dep_name);

    config_dnit(config);
    free(config);
    free(project_root);
    return 0;
}

static int cmd_dep_pull(int argc, char **argv)
{
    const char *specific_dep = NULL;
    if (argc >= 4)
    {
        specific_dep = argv[3];
    }

    char *project_root = find_project_root(".");
    if (!project_root)
    {
        fprintf(stderr, "error: not in a mach project (no mach.toml found)\n");
        return 1;
    }

    ProjectConfig *config = config_load_from_dir(project_root);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml\n");
        free(project_root);
        return 1;
    }

    if (config->dep_count == 0)
    {
        printf("no dependencies to pull\n");
        config_dnit(config);
        free(config);
        free(project_root);
        return 0;
    }

    int success = 1;
    for (int i = 0; i < config->dep_count; i++)
    {
        DepSpec *dep = config->deps[i];
        if (!dep || !dep->name)
            continue;

        // if specific dep requested, skip others
        if (specific_dep && strcmp(dep->name, specific_dep) != 0)
            continue;

        // only pull remote dependencies
        if (!dep->type || strcmp(dep->type, "remote") != 0)
        {
            printf("skipping local dependency '%s'\n", dep->name);
            continue;
        }

        char vendor_path[PATH_MAX];
        snprintf(vendor_path, sizeof(vendor_path), "dep/%s", dep->name);

        printf("pulling dependency '%s'...\n", dep->name);

        // initialize/update submodule
        char git_cmd[PATH_MAX * 2];
        snprintf(git_cmd, sizeof(git_cmd), "cd %s && git submodule update --init --recursive %s", project_root, vendor_path);

        if (run_git_command(git_cmd) != 0)
        {
            fprintf(stderr, "error: failed to update submodule for '%s'\n", dep->name);
            success = 0;
            continue;
        }

        // determine what to checkout based on version field
        char *resolved_ref = NULL;
        if (dep->version && strlen(dep->version) > 0)
        {
            // check if version starts with "branch/"
            if (strncmp(dep->version, "branch/", 7) == 0)
            {
                // branch reference
                const char *branch_name = dep->version + 7;
                printf("  checking out branch '%s'\n", branch_name);
                snprintf(git_cmd, sizeof(git_cmd), "cd %s/%s && git checkout %s && git pull origin %s", project_root, vendor_path, branch_name, branch_name);
                if (run_git_command(git_cmd) != 0)
                {
                    fprintf(stderr, "error: failed to checkout branch '%s' for '%s'\n", branch_name, dep->name);
                    success = 0;
                    continue;
                }
                resolved_ref = strdup(branch_name);
            }
            else
            {
                // try semver tag resolution first
                snprintf(git_cmd, sizeof(git_cmd), "cd %s/%s && git fetch --tags", project_root, vendor_path);
                if (run_git_command(git_cmd) != 0)
                {
                    fprintf(stderr, "warning: failed to fetch tags for '%s'\n", dep->name);
                }

                char tag_cmd[PATH_MAX * 2];
                snprintf(tag_cmd, sizeof(tag_cmd), "cd %s/%s && git tag -l", project_root, vendor_path);

                FILE *tag_pipe = popen(tag_cmd, "r");
                if (tag_pipe)
                {
                    char best_tag[256] = {0};
                    int  best_major = -1, best_minor = -1, best_patch = -1;
                    char line[256];

                    while (fgets(line, sizeof(line), tag_pipe))
                    {
                        size_t len = strlen(line);
                        if (len > 0 && line[len - 1] == '\n')
                            line[len - 1] = '\0';

                        int major, minor, patch;
                        if (parse_semver_tag(line, &major, &minor, &patch))
                        {
                            if (semver_matches(dep->version, major, minor, patch))
                            {
                                if (major > best_major || (major == best_major && minor > best_minor) || (major == best_major && minor == best_minor && patch > best_patch))
                                {
                                    best_major = major;
                                    best_minor = minor;
                                    best_patch = patch;
                                    strncpy(best_tag, line, sizeof(best_tag) - 1);
                                }
                            }
                        }
                    }
                    pclose(tag_pipe);

                    if (strlen(best_tag) > 0)
                    {
                        printf("  checking out tag '%s' for version constraint '%s'\n", best_tag, dep->version);
                        snprintf(git_cmd, sizeof(git_cmd), "cd %s/%s && git checkout %s", project_root, vendor_path, best_tag);
                        if (run_git_command(git_cmd) != 0)
                        {
                            fprintf(stderr, "error: failed to checkout tag '%s' for '%s'\n", best_tag, dep->name);
                            success = 0;
                            continue;
                        }
                        resolved_ref = strdup(best_tag);
                    }
                }

                // if no semver match, try as literal ref (tag name or commit hash)
                if (!resolved_ref)
                {
                    printf("  checking out ref '%s'\n", dep->version);
                    snprintf(git_cmd, sizeof(git_cmd), "cd %s/%s && git checkout %s", project_root, vendor_path, dep->version);
                    if (run_git_command(git_cmd) != 0)
                    {
                        fprintf(stderr, "error: failed to checkout ref '%s' for '%s'\n", dep->version, dep->name);
                        success = 0;
                        continue;
                    }
                    resolved_ref = strdup(dep->version);
                }
            }
        }
        else
        {
            // no version specified, get latest commit hash and store it
            char hash_cmd[PATH_MAX * 2];
            snprintf(hash_cmd, sizeof(hash_cmd), "cd %s/%s && git rev-parse HEAD", project_root, vendor_path);

            FILE *hash_pipe = popen(hash_cmd, "r");
            if (hash_pipe)
            {
                char commit_hash[256];
                if (fgets(commit_hash, sizeof(commit_hash), hash_pipe))
                {
                    size_t len = strlen(commit_hash);
                    if (len > 0 && commit_hash[len - 1] == '\n')
                        commit_hash[len - 1] = '\0';

                    printf("  pinning to commit '%s'\n", commit_hash);
                    resolved_ref = strdup(commit_hash);

                    // update the config with the resolved commit
                    free(dep->version);
                    dep->version = strdup(commit_hash);
                }
                pclose(hash_pipe);
            }

            if (!resolved_ref)
            {
                fprintf(stderr, "warning: failed to resolve commit hash for '%s'\n", dep->name);
            }
        }

        if (resolved_ref)
        {
            free(resolved_ref);
        }

        printf("  dependency '%s' pulled successfully\n", dep->name);
    }

    // save updated config with resolved versions
    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s/mach.toml", project_root);
    if (!config_save(config, config_path))
    {
        fprintf(stderr, "warning: failed to save updated dependency versions\n");
    }

    config_dnit(config);
    free(config);
    free(project_root);
    return success ? 0 : 1;
}

// helper function implementations

static int run_git_command(const char *command)
{
    int result = system(command);
    return WIFEXITED(result) ? WEXITSTATUS(result) : 1;
}

static int vendor_dir_exists(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static char *find_project_root(const char *start_path)
{
    return fs_find_project_root(start_path);
}

static int config_add_dep_and_save(ProjectConfig *config, const char *config_path, DepSpec *dep)
{
    // add to config
    config->deps = realloc(config->deps, sizeof(DepSpec *) * (config->dep_count + 1));
    if (!config->deps)
        return -1;

    config->deps[config->dep_count] = dep;
    config->dep_count++;

    // save config
    return config_save(config, config_path) ? 0 : -1;
}

static int config_remove_dep_and_save(ProjectConfig *config, const char *config_path, const char *dep_name)
{
    int found = -1;
    for (int i = 0; i < config->dep_count; i++)
    {
        if (config->deps[i] && config->deps[i]->name && strcmp(config->deps[i]->name, dep_name) == 0)
        {
            found = i;
            break;
        }
    }

    if (found < 0)
        return -1;

    // free the dep
    DepSpec *dep = config->deps[found];
    if (dep)
    {
        free(dep->name);
        free(dep->type);
        free(dep->path);
        free(dep->version);
        free(dep);
    }

    // shift array
    for (int i = found; i < config->dep_count - 1; i++)
    {
        config->deps[i] = config->deps[i + 1];
    }
    config->dep_count--;

    // save config
    return config_save(config, config_path) ? 0 : -1;
}

static int parse_semver_tag(const char *tag, int *major, int *minor, int *patch)
{
    *major = *minor = *patch = 0;

    const char *ptr = tag;
    // skip leading 'v' if present
    if (*ptr == 'v' || *ptr == 'V')
        ptr++;

    // parse major.minor.patch
    if (sscanf(ptr, "%d.%d.%d", major, minor, patch) == 3)
        return 1;

    return 0;
}

static int semver_matches(const char *version_constraint, int major, int minor, int patch)
{
    // simple matching: exact version or prefix match
    // supports: "1.2.3", "v1.2.3", "^1.0.0" (compatible with 1.x.x), "~1.2.0" (compatible with 1.2.x)

    const char *ptr    = version_constraint;
    char        prefix = 0;

    if (*ptr == '^' || *ptr == '~')
    {
        prefix = *ptr;
        ptr++;
    }

    if (*ptr == 'v' || *ptr == 'V')
        ptr++;

    int req_major = 0, req_minor = 0, req_patch = 0;
    if (sscanf(ptr, "%d.%d.%d", &req_major, &req_minor, &req_patch) < 1)
        return 0; // invalid constraint

    if (prefix == '^')
    {
        // caret: compatible with same major version
        return major == req_major && (minor > req_minor || (minor == req_minor && patch >= req_patch));
    }
    else if (prefix == '~')
    {
        // tilde: compatible with same major.minor version
        return major == req_major && minor == req_minor && patch >= req_patch;
    }
    else
    {
        // exact match
        return major == req_major && minor == req_minor && patch == req_patch;
    }
}
