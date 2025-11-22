#include "filesystem.h"
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
extern char *realpath(const char *restrict path, char *restrict resolved_path);
#endif

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#define mkdir(path, mode) _mkdir(path)
#define stat _stat
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)

// windows realpath equivalent
static char *realpath_windows(const char *path, char *resolved)
{
    char buffer[PATH_MAX];
    if (!_fullpath(buffer, path, PATH_MAX))
        return NULL;

    if (resolved)
    {
        strcpy(resolved, buffer);
        return resolved;
    }
    return strdup(buffer);
}
#define realpath realpath_windows
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

// os-independent path join (uses correct path separator for platform)
char *fs_path_join(const char *first, ...)
{
    if (!first)
        return NULL;

    char *result = strdup(first);
    if (!result)
        return NULL;

    va_list args;
    va_start(args, first);

    const char *part;
    while ((part = va_arg(args, const char *)) != NULL)
    {
        size_t len1 = strlen(result);
        size_t len2 = strlen(part);
        int    need_sep = (len1 > 0 && result[len1 - 1] != '/' && result[len1 - 1] != '\\' &&
                           len2 > 0 && part[0] != '/' && part[0] != '\\');

        char *new_result = malloc(len1 + len2 + (need_sep ? 2 : 1));
        if (!new_result)
        {
            free(result);
            va_end(args);
            return NULL;
        }

        strcpy(new_result, result);
        if (need_sep)
        {
#ifdef _WIN32
            strcat(new_result, "\\");
#else
            strcat(new_result, "/");
#endif
        }
        strcat(new_result, part);

        free(result);
        result = new_result;
    }

    va_end(args);
    return result;
}

int fs_chdir(const char *path)
{
    if (!path)
        return 0;
#ifdef _WIN32
    return _chdir(path) == 0;
#else
    return chdir(path) == 0;
#endif
}

char *fs_read_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
        return NULL;
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

int fs_file_exists(const char *path)
{
    if (!path)
        return 0;
    struct stat st;
    return stat(path, &st) == 0;
}

int fs_is_directory(const char *path)
{
    if (!path)
        return 0;
    struct stat st;
    if (stat(path, &st) != 0)
        return 0;
    return S_ISDIR(st.st_mode);
}

int fs_is_mach_file(const char *path)
{
    if (!path)
        return 0;
    size_t len = strlen(path);
    if (len < 6) // minimum: "a.mach"
        return 0;
    return strcmp(path + len - 5, ".mach") == 0;
}

int fs_ensure_dir_recursive(const char *path)
{
    if (!path || !*path)
        return 1;

    char *copy = strdup(path);
    if (!copy)
        return 0;

    for (char *p = copy + 1; *p; ++p)
    {
        if (*p == '/')
        {
            *p = '\0';
            mkdir(copy, 0755);
            *p = '/';
        }
    }
    int r = mkdir(copy, 0755);
    (void)r;
    free(copy);
    return 1;
}

char *fs_dirname(const char *path)
{
    const char *slash = strrchr(path, '/');
    if (!slash)
        return strdup(".");
    size_t len = (size_t)(slash - path);
    char  *out = malloc(len + 1);
    memcpy(out, path, len);
    out[len] = '\0';
    return out;
}

char *fs_find_project_root(const char *start_path)
{
    char resolved[PATH_MAX];
    if (realpath(start_path, resolved))
    {
        struct stat st;
        if (stat(resolved, &st) != 0)
            return strdup(".");

        if (!S_ISDIR(st.st_mode))
        {
            char *slash = strrchr(resolved, '/');
            if (slash)
                *slash = '\0';
        }

        if (resolved[0] == '\0')
            strcpy(resolved, "/");

        char *dir = strdup(resolved);
        if (!dir)
            return NULL;

        for (int depth = 0; depth < 64; depth++)
        {
            char cfg[PATH_MAX];
            snprintf(cfg, sizeof(cfg), "%s/mach.toml", dir);
            if (fs_file_exists(cfg))
                return dir;

            if (strcmp(dir, "/") == 0 || dir[0] == '\0')
                break;

            char *slash = strrchr(dir, '/');
            if (!slash)
            {
                dir[0] = '\0';
            }
            else if (slash == dir)
            {
                slash[1] = '\0';
            }
            else
            {
                *slash = '\0';
            }
        }

        free(dir);
        return strdup(".");
    }

    // fallback to relative walk if realpath fails
    char *dir = fs_dirname(start_path);
    for (int i = 0; i < 16 && dir; i++)
    {
        char cfg[1024];
        snprintf(cfg, sizeof(cfg), "%s/mach.toml", dir);
        if (fs_file_exists(cfg))
            return dir;
        const char *slash = strrchr(dir, '/');
        if (!slash)
        {
            free(dir);
            return strdup(".");
        }
        if (slash == dir)
        {
            dir[1] = '\0';
            return dir;
        }
        size_t nlen = (size_t)(slash - dir);
        char  *up   = malloc(nlen + 1);
        memcpy(up, dir, nlen);
        up[nlen] = '\0';
        free(dir);
        dir = up;
    }
    return dir;
}

char *fs_get_base_filename(const char *path)
{
    const char *last_slash = strrchr(path, '/');
    const char *filename   = last_slash ? last_slash + 1 : path;
    const char *last_dot   = strrchr(filename, '.');
    if (!last_dot)
        return strdup(filename);
    size_t len  = (size_t)(last_dot - filename);
    char  *base = malloc(len + 1);
    strncpy(base, filename, len);
    base[len] = '\0';
    return base;
}

#ifndef _WIN32
#include <dirent.h>
#endif

static void fs_list_mach_files_recursive_impl(const char *dir_path, char ***files, int *count, int *capacity)
{
#ifdef _WIN32
    // windows directory traversal using FindFirstFile/FindNextFile
    WIN32_FIND_DATA find_data;
    char search_path[PATH_MAX];
    snprintf(search_path, sizeof(search_path), "%s\\*", dir_path);
    
    HANDLE handle = FindFirstFile(search_path, &find_data);
    if (handle == INVALID_HANDLE_VALUE)
        return;
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
            continue;
        
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s\\%s", dir_path, find_data.cFileName);
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // recurse into subdirectory
            fs_list_mach_files_recursive_impl(full_path, files, count, capacity);
        }
        else
        {
            // check if it's a .mach file
            size_t len = strlen(find_data.cFileName);
            if (len > 5 && strcmp(find_data.cFileName + len - 5, ".mach") == 0)
            {
                if (*count >= *capacity)
                {
                    *capacity = *capacity == 0 ? 16 : *capacity * 2;
                    *files = realloc(*files, sizeof(char*) * (*capacity + 1));
                }
                (*files)[*count] = strdup(full_path);
                (*count)++;
            }
        }
    } while (FindNextFile(handle, &find_data));
    
    FindClose(handle);
#else
    // unix directory traversal using opendir/readdir
    DIR *dir = opendir(dir_path);
    if (!dir)
        return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) != 0)
            continue;
        
        if (S_ISDIR(st.st_mode))
        {
            // recurse into subdirectory
            fs_list_mach_files_recursive_impl(full_path, files, count, capacity);
        }
        else if (S_ISREG(st.st_mode))
        {
            // check if it's a .mach file
            size_t len = strlen(entry->d_name);
            if (len > 5 && strcmp(entry->d_name + len - 5, ".mach") == 0)
            {
                if (*count >= *capacity)
                {
                    *capacity = *capacity == 0 ? 16 : *capacity * 2;
                    *files = realloc(*files, sizeof(char*) * (*capacity + 1));
                }
                (*files)[*count] = strdup(full_path);
                (*count)++;
            }
        }
    }
    
    closedir(dir);
#endif
}
