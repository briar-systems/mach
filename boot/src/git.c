#include "git.h"
#include "filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// execute a git command and return exit code
int git_exec(const char *command)
{
    if (!command)
    {
        return -1;
    }
    return system(command);
}

// check if a directory is a git repository
bool git_is_repo(const char *path)
{
    if (!path)
    {
        return false;
    }

    char *git_dir = path_join(path, ".git");
    if (!git_dir)
    {
        return false;
    }

    bool exists = file_exists(git_dir) || is_directory(git_dir);
    free(git_dir);
    return exists;
}

// initialize a git submodule at the specified path with the given URL
bool git_submodule_init(const char *path, const char *url)
{
    if (!path || !url)
    {
        return false;
    }

    // construct: git submodule add <url> <path>
    size_t cmd_len = strlen("git submodule add \"\" \"\"") + strlen(url) + strlen(path) + 1;
    char  *cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }

    snprintf(cmd, cmd_len, "git submodule add \"%s\" \"%s\"", url, path);
    int result = git_exec(cmd);
    free(cmd);

    if (result != 0)
    {
        return false;
    }

    // initialize and update the submodule
    cmd_len = strlen("git submodule update --init \"\"") + strlen(path) + 1;
    cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }

    snprintf(cmd, cmd_len, "git submodule update --init \"%s\"", path);
    result = git_exec(cmd);
    free(cmd);

    return result == 0;
}

// update a git submodule at the specified path
bool git_submodule_update(const char *path)
{
    if (!path)
    {
        return false;
    }

    size_t cmd_len = strlen("git submodule update --init --recursive \"\"") + strlen(path) + 1;
    char  *cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }

    snprintf(cmd, cmd_len, "git submodule update --init --recursive \"%s\"", path);
    int result = git_exec(cmd);
    free(cmd);

    return result == 0;
}

// fetch latest changes for a submodule
bool git_submodule_fetch(const char *path)
{
    if (!path)
    {
        return false;
    }

    char *cwd = get_current_dir();
    if (!cwd)
    {
        return false;
    }

    if (!chdir_path(path))
    {
        free(cwd);
        return false;
    }

    int result = git_exec("git fetch --all --tags");

    chdir_path(cwd);
    free(cwd);
    return result == 0;
}

// checkout a specific version (branch, tag, or commit) in a submodule
bool git_submodule_checkout(const char *path, const char *version)
{
    if (!path || !version)
    {
        return false;
    }

    char *cwd = get_current_dir();
    if (!cwd)
    {
        return false;
    }

    if (!chdir_path(path))
    {
        free(cwd);
        return false;
    }

    size_t cmd_len = strlen("git checkout \"\"") + strlen(version) + 1;
    char  *cmd     = malloc(cmd_len);
    if (!cmd)
    {
        chdir_path(cwd);
        free(cwd);
        return false;
    }

    snprintf(cmd, cmd_len, "git checkout \"%s\"", version);
    int result = git_exec(cmd);
    free(cmd);

    chdir_path(cwd);
    free(cwd);
    return result == 0;
}

// remove a git submodule completely
bool git_submodule_remove(const char *path)
{
    if (!path)
    {
        return false;
    }

    // deinit the submodule
    size_t cmd_len = strlen("git submodule deinit -f \"\"") + strlen(path) + 1;
    char  *cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }

    snprintf(cmd, cmd_len, "git submodule deinit -f \"%s\"", path);
    int result = git_exec(cmd);
    free(cmd);

    if (result != 0)
    {
        return false;
    }

    // remove from git index
    cmd_len = strlen("git rm -f \"\"") + strlen(path) + 1;
    cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }

    snprintf(cmd, cmd_len, "git rm -f \"%s\"", path);
    result = git_exec(cmd);
    free(cmd);

    if (result != 0)
    {
        return false;
    }

    // remove cached git data from .git/modules/<path>
    char *modules_path = path_join(".git/modules", path);
    if (modules_path)
    {
        remove_directory_recursive(modules_path);
        free(modules_path);
    }

    return true;
}

// parse version string and checkout appropriate ref
// handles: branch/name, commit/hash, semver patterns (^1.2.3, ~1.2.3, 1.2.3)
bool git_checkout_version(const char *path, const char *version)
{
    if (!path || !version)
    {
        return false;
    }

    char *ref_to_checkout = NULL;

    // check for branch/ prefix
    if (strncmp(version, "branch/", 7) == 0)
    {
        ref_to_checkout = strdup(version + 7);
    }
    // check for commit/ prefix
    else if (strncmp(version, "commit/", 7) == 0)
    {
        ref_to_checkout = strdup(version + 7);
    }
    // handle semver patterns
    else if (version[0] == '^' || version[0] == '~' || 
             (version[0] >= '0' && version[0] <= '9'))
    {
        // for semver patterns, we'll try to match tags
        // strip prefix characters
        const char *tag = version;
        if (version[0] == '^' || version[0] == '~')
        {
            tag = version + 1;
        }

        // try both with and without 'v' prefix
        size_t tag_len = strlen("v") + strlen(tag) + 1;
        char  *v_tag   = malloc(tag_len);
        if (!v_tag)
        {
            return false;
        }
        snprintf(v_tag, tag_len, "v%s", tag);

        // first try with 'v' prefix
        if (git_submodule_checkout(path, v_tag))
        {
            free(v_tag);
            return true;
        }

        // then try without 'v' prefix
        free(v_tag);
        ref_to_checkout = strdup(tag);
    }
    else
    {
        ref_to_checkout = strdup(version);
    }

    if (!ref_to_checkout)
    {
        return false;
    }

    bool result = git_submodule_checkout(path, ref_to_checkout);
    free(ref_to_checkout);

    return result;
}
