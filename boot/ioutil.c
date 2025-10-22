#include "ioutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

bool is_directory(char *path)
{
#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat path_stat;
    if (stat(path, &path_stat) != 0)
    {
        return false;
    }
    return S_ISDIR(path_stat.st_mode);
#endif
}

bool file_exists(char *path)
{
#ifdef _WIN32
    wchar_t wpath[MAX_PATH];
    mbstowcs(wpath, path, MAX_PATH);
    DWORD attributes = GetFileAttributesW(wpath);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat path_stat;
    return stat(path, &path_stat) == 0 && !S_ISDIR(path_stat.st_mode);
#endif
}

bool path_is_absolute(char *path)
{
#ifdef _WIN32
    // "C:\ or C:/"
    return (path[1] == ':' && (path[2] == '\\' || path[2] == '/'));
#else
    // "/home/user"
    return path[0] == '/';
#endif
}

char *path_dirname(char *path)
{
#ifdef _WIN32
    char *last_slash = strrchr(path, '\\');
#else
    char *last_slash = strrchr(path, '/');
#endif

    if (last_slash == NULL)
    {
        return ".";
    }

    int   len     = last_slash - path;
    char *dirname = malloc(len + 1);
    strncpy(dirname, path, len);
    dirname[len] = '\0';
    return dirname;
}

char *path_lastname(char *path)
{
#ifdef _WIN32
    char *last_slash = strrchr(path, '\\');
#else
    char *last_slash = strrchr(path, '/');
#endif

    if (last_slash == NULL)
    {
        return path;
    }

    return last_slash + 1;
}

char *path_join(char *a, char *b)
{
    int   len_a  = strlen(a);
    int   len_b  = strlen(b);
    char *joined = malloc(len_a + len_b + 2);
    strcpy(joined, a);

#ifdef _WIN32
    if (a[len_a - 1] != '\\' && a[len_a - 1] != '/')
    {
        strcat(joined, "\\");
    }
#else
    if (a[len_a - 1] != '/')
    {
        strcat(joined, "/");
    }
#endif

    strcat(joined, b);
    return joined;
}

// subtract the base path from the path
// e.g:
// - base =   "C:\project_root"
// - path =   "C:\project_root\src\main.mach"
// - result = "src\main.mach"
char *path_relative(char *base, char *path)
{
    int len_base = strlen(base);
    int len_path = strlen(path);

    if (len_path < len_base)
    {
        return NULL;
    }

    if (strncmp(base, path, len_base) != 0)
    {
        return NULL;
    }

    if (path[len_base] == '\\' || path[len_base] == '/')
    {
        return path + len_base + 1;
    }

    return path + len_base;
}

char *path_get_extension(char *path)
{
    char *last_dot = strrchr(path, '.');
    if (last_dot == NULL)
    {
        return NULL;
    }

    return last_dot + 1;
}

char *read_file(char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

char **list_files(char *path)
{
    char **file_list = NULL;
    int    count     = 0;

#ifdef _WIN32
    WIN32_FIND_DATAW find_file_data;
    HANDLE           h_find = INVALID_HANDLE_VALUE;
    wchar_t          search_path[MAX_PATH];

    // Convert the input path to a wide character string
    size_t   path_len = strlen(path) + 1;
    wchar_t *wpath    = malloc(path_len * sizeof(wchar_t));
    mbstowcs(wpath, path, path_len);

    // Append the wildcard to the path
    swprintf(search_path, MAX_PATH, L"%s\\*", wpath);
    free(wpath);

    h_find = FindFirstFileW(search_path, &find_file_data);

    if (h_find == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "FindFirstFileW failed with error code %lu\n", GetLastError());
        return NULL;
    }

    do
    {
        if (wcscmp(find_file_data.cFileName, L".") != 0 && wcscmp(find_file_data.cFileName, L"..") != 0)
        {
            file_list = realloc(file_list, sizeof(char *) * (count + 1));
            if (file_list == NULL)
            {
                perror("realloc");
                FindClose(h_find);
                return NULL;
            }

            size_t len       = wcslen(find_file_data.cFileName) + 1;
            file_list[count] = malloc(len * sizeof(char));
            if (file_list[count] == NULL)
            {
                perror("malloc");
                FindClose(h_find);
                return NULL;
            }

            wcstombs(file_list[count], find_file_data.cFileName, len);

            count++;
        }
    } while (FindNextFileW(h_find, &find_file_data) != 0);

    FindClose(h_find);

    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        fprintf(stderr, "FindNextFileW failed with error code %lu\n", GetLastError());
        for (int i = 0; i < count; i++)
        {
            free(file_list[i]);
        }
        free(file_list);
        return NULL;
    }

#else
    DIR           *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir");
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            file_list = realloc(file_list, sizeof(char *) * (count + 1));
            if (file_list == NULL)
            {
                perror("realloc");
                closedir(dir);
                return NULL;
            }

            file_list[count] = strdup(entry->d_name);
            if (file_list[count] == NULL)
            {
                perror("strdup");
                closedir(dir);
                return NULL;
            }

            count++;
        }
    }

    closedir(dir);
#endif

    file_list = realloc(file_list, sizeof(char *) * (count + 1));
    if (file_list == NULL)
    {
        perror("realloc");
        return NULL;
    }
    file_list[count] = NULL;

    return file_list;
}

char **list_files_recursive(char *path, char **file_list, int count)
{
#ifdef _WIN32
    WIN32_FIND_DATAA find_file_data;
    HANDLE           h_find = NULL;
    char             search_path[MAX_PATH];

    snprintf(search_path, MAX_PATH, "%s\\*", path);

    h_find = FindFirstFileA(search_path, &find_file_data);
    if (h_find == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "FindFirstFile failed with error code %lu\n", GetLastError());
        return NULL;
    }

    do
    {
        if (strcmp(find_file_data.cFileName, ".") != 0 && strcmp(find_file_data.cFileName, "..") != 0)
        {
            char *full_path = path_join(path, find_file_data.cFileName);
            if (is_directory(full_path))
            {
                char **subdir_files = list_files_recursive(full_path, NULL, 0);
                if (subdir_files == NULL)
                {
                    FindClose(h_find);
                    return NULL;
                }

                for (int i = 0; subdir_files[i] != NULL; i++)
                {
                    file_list = realloc(file_list, sizeof(char *) * (count + 1));
                    if (file_list == NULL)
                    {
                        perror("realloc");
                        FindClose(h_find);
                        return NULL;
                    }

                    char *full_subdir_path = path_join(find_file_data.cFileName, subdir_files[i]);

                    file_list[count] = full_subdir_path;
                    count++;
                }
            }
            else
            {
                file_list = realloc(file_list, sizeof(char *) * (count + 1));
                if (file_list == NULL)
                {
                    perror("realloc");
                    FindClose(h_find);
                    return NULL;
                }

                file_list[count] = strdup(find_file_data.cFileName);
                if (file_list[count] == NULL)
                {
                    perror("strdup");
                    FindClose(h_find);
                    return NULL;
                }

                count++;
            }

            free(full_path);
        }
    } while (FindNextFileA(h_find, &find_file_data) != 0);

    FindClose(h_find);

    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        fprintf(stderr, "FindNextFileW failed with error code %lu\n", GetLastError());
        for (int i = 0; i < count; i++)
        {
            free(file_list[i]);
        }
        free(file_list);
        return NULL;
    }

#else
    DIR           *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir");
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char *full_path = path_join(path, entry->d_name);
            if (is_directory(full_path))
            {
                file_list = list_files_recursive(full_path, file_list, count);
                if (file_list == NULL)
                {
                    closedir(dir);
                    return NULL;
                }
            }
            else
            {
                file_list = realloc(file_list, sizeof(char *) * (count + 1));
                if (file_list == NULL)
                {
                    perror("realloc");
                    closedir(dir);
                    return NULL;
                }

                file_list[count] = strdup(entry->d_name);
                if (file_list[count] == NULL)
                {
                    perror("strdup");
                    closedir(dir);
                    return NULL;
                }

                count++;
            }

            free(full_path);
        }
    }

    closedir(dir);
#endif

    file_list = realloc(file_list, sizeof(char *) * (count + 1));
    if (file_list == NULL)
    {
        perror("realloc");
        return NULL;
    }
    file_list[count] = NULL;

    return file_list;
}
