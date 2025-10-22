#include "filesystem.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
