#ifndef GIT_H
#define GIT_H

#include <stdbool.h>

// execute a git command and return exit code
int git_exec(const char *command);

// check if a directory is a git repository
bool git_is_repo(const char *path);

// initialize a git submodule at the specified path with the given URL
bool git_submodule_init(const char *path, const char *url);

// update a git submodule at the specified path
bool git_submodule_update(const char *path);

// fetch latest changes for a submodule
bool git_submodule_fetch(const char *path);

// checkout a specific version (branch, tag, or commit) in a submodule
bool git_submodule_checkout(const char *path, const char *version);

// remove a git submodule completely
bool git_submodule_remove(const char *path);

// parse version string and checkout appropriate ref
// handles: branch/name, commit/hash, semver patterns
bool git_checkout_version(const char *path, const char *version);

#endif
