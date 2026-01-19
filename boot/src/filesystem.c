#include "filesystem.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#define PATH_SEP '\\'
#define mkdir(path, mode) _mkdir(path)
#define stat _stat
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH_SEP '/'
extern char *realpath(const char *restrict path, char *restrict resolved_path);
#endif

// helper: check if character is a path separator
bool is_sep(char c)
{
    return c == '/' || c == '\\';
}

// path_join: join two path components
char *path_join(const char *a, const char *b)
{
    if (!a || !b)
    {
        return NULL;
    }

    size_t len_a    = strlen(a);
    size_t len_b    = strlen(b);
    bool   need_sep = len_a > 0 && !is_sep(a[len_a - 1]) && !is_sep(b[0]);
    size_t total    = len_a + len_b + (need_sep ? 1 : 0) + 1;

    char *result = malloc(total);
    if (!result)
    {
        return NULL;
    }

    strcpy(result, a);
    if (need_sep)
    {
        result[len_a] = PATH_SEP;
        strcpy(result + len_a + 1, b);
    }
    else
    {
        strcpy(result + len_a, b);
    }

    return result;
}

// path_dirname: return directory portion of path
char *path_dirname(const char *path)
{
    if (!path)
    {
        return strdup(".");
    }

    const char *last_sep = NULL;
    for (const char *p = path; *p; p++)
    {
        if (is_sep(*p))
        {
            last_sep = p;
        }
    }

    if (!last_sep)
    {
        return strdup(".");
    }

    size_t len = (size_t)(last_sep - path);
    if (len == 0)
    {
        char root[2] = {PATH_SEP, '\0'};
        return strdup(root);
    }

    char *result = malloc(len + 1);
    if (result)
    {
        memcpy(result, path, len);
        result[len] = '\0';
    }
    return result;
}

// path_basename: return filename portion of path
char *path_basename(const char *path)
{
    if (!path)
    {
        return NULL;
    }

    const char *last_sep = NULL;
    for (const char *p = path; *p; p++)
    {
        if (is_sep(*p))
        {
            last_sep = p;
        }
    }

    const char *basename = last_sep ? last_sep + 1 : path;
    return strdup(basename);
}

// path_extension: return file extension including dot (or NULL)
char *path_extension(const char *path)
{
    if (!path)
    {
        return NULL;
    }

    const char *last_sep = NULL;
    const char *last_dot = NULL;

    for (const char *p = path; *p; p++)
    {
        if (is_sep(*p))
        {
            last_sep = p;
            last_dot = NULL;
        }
        else if (*p == '.')
        {
            last_dot = p;
        }
    }

    if (!last_dot || (last_sep && last_dot < last_sep))
    {
        return NULL;
    }

    return strdup(last_dot);
}

// path_is_absolute: check if path is absolute
bool path_is_absolute(const char *path)
{
    if (!path || !path[0])
    {
        return false;
    }

#ifdef _WIN32
    // drive letter or UNC path
    char c = path[0];
    if (((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && path[1] == ':')
    {
        return true;
    }
    return is_sep(path[0]) && is_sep(path[1]);
#else
    return is_sep(path[0]);
#endif
}

// file_exists: check if file or directory exists
bool file_exists(const char *path)
{
    if (!path)
    {
        return false;
    }
    struct stat st;
    return stat(path, &st) == 0;
}

// is_directory: check if path is a directory
bool is_directory(const char *path)
{
    if (!path)
    {
        return false;
    }
    struct stat st;
    if (stat(path, &st) != 0)
    {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

// read_file: read entire file into memory (binary safe)
char *read_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size < 0)
    {
        fclose(file);
        return NULL;
    }

    fseek(file, 0, SEEK_SET);
    char *buf = malloc(size + 1);
    if (!buf)
    {
        fclose(file);
        return NULL;
    }

    size_t n = fread(buf, 1, size, file);
    buf[n]   = '\0';
    fclose(file);
    return buf;
}

// chdir_path: change current working directory
bool chdir_path(const char *path)
{
    if (!path)
    {
        return false;
    }
#ifdef _WIN32
    return _chdir(path) == 0;
#else
    return chdir(path) == 0;
#endif
}

// get_current_dir: get current working directory
char *get_current_dir(void)
{
    char buffer[PATH_MAX];
#ifdef _WIN32
    if (!_getcwd(buffer, PATH_MAX))
    {
        return NULL;
    }
#else
    if (!getcwd(buffer, PATH_MAX))
    {
        return NULL;
    }
#endif
    return strdup(buffer);
}

// remove_directory_recursive: remove directory and all contents
bool remove_directory_recursive(const char *path)
{
    if (!path)
    {
        return false;
    }

#ifdef _WIN32
    size_t cmd_len = strlen("rmdir /S /Q \"\"") + strlen(path) + 1;
    char  *cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }
    snprintf(cmd, cmd_len, "rmdir /S /Q \"%s\"", path);
#else
    size_t cmd_len = strlen("rm -rf \"\"") + strlen(path) + 1;
    char  *cmd     = malloc(cmd_len);
    if (!cmd)
    {
        return false;
    }
    snprintf(cmd, cmd_len, "rm -rf \"%s\"", path);
#endif

    int result = system(cmd);
    free(cmd);
    return result == 0;
}

// ensure_dir_recursive: create directory and all parents
bool ensure_dir_recursive(const char *path)
{
    if (!path || !*path)
    {
        return true;
    }

    char *copy = strdup(path);
    if (!copy)
    {
        return false;
    }

    // normalize separators
    for (char *p = copy; *p; p++)
    {
        if (is_sep(*p))
        {
            *p = PATH_SEP;
        }
    }

    // create each directory in the path
    for (char *p = copy + 1; *p; p++)
    {
        if (*p == PATH_SEP)
        {
            *p = '\0';
            mkdir(copy, 0755);
            *p = PATH_SEP;
        }
    }

    int result = mkdir(copy, 0755);
    (void)result;
    free(copy);
    return true;
}

// list_files: list files in directory (non-recursive)
char **list_files(const char *path)
{
    if (!path)
    {
        return NULL;
    }

#ifdef _WIN32
    char pattern[PATH_MAX];
    snprintf(pattern, sizeof(pattern), "%s\\*", path);

    WIN32_FIND_DATAA find_data;
    HANDLE           handle = FindFirstFileA(pattern, &find_data);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    int    capacity = 16;
    int    count    = 0;
    char **files    = malloc(sizeof(char *) * capacity);
    if (!files)
    {
        FindClose(handle);
        return NULL;
    }

    do
    {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
        {
            continue;
        }

        if (count >= capacity - 1)
        {
            capacity *= 2;
            char **new_files = realloc(files, sizeof(char *) * capacity);
            if (!new_files)
            {
                free_string_array(files);
                FindClose(handle);
                return NULL;
            }
            files = new_files;
        }

        char *joined = path_join(path, find_data.cFileName);
        if (joined)
        {
            files[count++] = joined;
        }
    } while (FindNextFileA(handle, &find_data));

    FindClose(handle);
    files[count] = NULL;
    return files;
#else
    DIR *dir = opendir(path);
    if (!dir)
    {
        return NULL;
    }

    int    capacity = 16;
    int    count    = 0;
    char **files    = malloc(sizeof(char *) * capacity);
    if (!files)
    {
        closedir(dir);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        if (count >= capacity - 1)
        {
            capacity *= 2;
            char **new_files = realloc(files, sizeof(char *) * capacity);
            if (!new_files)
            {
                free_string_array(files);
                closedir(dir);
                return NULL;
            }
            files = new_files;
        }

        char *joined = path_join(path, entry->d_name);
        if (joined)
        {
            files[count++] = joined;
        }
    }

    closedir(dir);
    files[count] = NULL;
    return files;
#endif
}

// helper for list_files_recursive
static void list_files_recursive_impl(const char *path, char ***files, int *count, int *capacity)
{
    char **entries = list_files(path);
    if (!entries)
    {
        return;
    }

    for (int i = 0; entries[i]; i++)
    {
        if (is_directory(entries[i]))
        {
            list_files_recursive_impl(entries[i], files, count, capacity);
        }
        else
        {
            if (*count >= *capacity - 1)
            {
                *capacity *= 2;
                char **new_files = realloc(*files, sizeof(char *) * (*capacity));
                if (!new_files)
                {
                    free_string_array(entries);
                    return;
                }
                *files = new_files;
            }
            (*files)[(*count)++] = strdup(entries[i]);
        }
    }

    free_string_array(entries);
}

// list_files_recursive: list all files recursively
char **list_files_recursive(const char *path)
{
    int    capacity = 64;
    int    count    = 0;
    char **files    = malloc(sizeof(char *) * capacity);
    if (!files)
    {
        return NULL;
    }

    list_files_recursive_impl(path, &files, &count, &capacity);
    files[count] = NULL;
    return files;
}

// free_string_array: free NULL-terminated array of strings
void free_string_array(char **array)
{
    if (!array)
    {
        return;
    }
    for (int i = 0; array[i]; i++)
    {
        free(array[i]);
    }
    free(array);
}

// absolutize_path: convert relative path to absolute path
char *absolutize_path(const char *path)
{
    if (!path)
    {
        return NULL;
    }

    char resolved[PATH_MAX];

#ifdef _WIN32
    if (!_fullpath(resolved, path, PATH_MAX))
    {
        return NULL;
    }
#else
    if (!realpath(path, resolved))
    {
        return NULL;
    }
#endif

    return strdup(resolved);
}

// find_project_root: search up directory tree for mach.toml
char *find_project_root(const char *start_path)
{
    char *resolved = absolutize_path(start_path);
    if (!resolved)
    {
        return strdup(".");
    }

    struct stat st;
    if (stat(resolved, &st) != 0)
    {
        free(resolved);
        return strdup(".");
    }

    // if it's a file, get its directory
    if (!S_ISDIR(st.st_mode))
    {
        char *dir = path_dirname(resolved);
        free(resolved);
        if (!dir)
        {
            return strdup(".");
        }
        resolved = dir;
    }

    // search up the tree
    for (int depth = 0; depth < 64; depth++)
    {
        char *config_path = path_join(resolved, "mach.toml");
        if (config_path && file_exists(config_path))
        {
            free(config_path);
            return resolved;
        }
        free(config_path);

        // check if we're at root
#ifdef _WIN32
        if (strlen(resolved) <= 3)
        {
            break; // "C:\" or similar
        }
#else
        if (strcmp(resolved, "/") == 0)
        {
            break;
        }
#endif

        char *parent = path_dirname(resolved);
        if (!parent || strcmp(parent, resolved) == 0)
        {
            free(parent);
            break;
        }

        free(resolved);
        resolved = parent;
    }

    free(resolved);
    return strdup(".");
}

// is_mach_file: check if path ends with .mach
bool is_mach_file(const char *path)
{
    if (!path)
    {
        return false;
    }
    size_t len = strlen(path);
    if (len < 6)
    {
        return false;
    }
    return strcmp(path + len - 5, ".mach") == 0;
}

// get_base_filename: return filename without extension
char *get_base_filename(const char *path)
{
    if (!path)
    {
        return NULL;
    }

    char *base = path_basename(path);
    if (!base)
    {
        return NULL;
    }

    char *dot = strrchr(base, '.');
    if (dot)
    {
        *dot = '\0';
    }

    return base;
}
