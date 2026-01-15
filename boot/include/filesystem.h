#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdbool.h>

// simple string-based path utilities (cross-platform)
char *path_join(const char *a, const char *b);
char *path_dirname(const char *path);
char *path_basename(const char *path);
char *path_extension(const char *path);
bool  path_is_absolute(const char *path);
char *absolutize_path(const char *path);

// file/directory queries
bool file_exists(const char *path);
bool is_directory(const char *path);

// file operations
char *read_file(const char *path);
bool  chdir_path(const char *path);
char *get_current_dir(void);
bool  ensure_dir_recursive(const char *path);
bool  remove_directory_recursive(const char *path);

// directory listing
char **list_files(const char *path);
char **list_files_recursive(const char *path);
void   free_string_array(char **array);

// project-specific utilities
char *find_project_root(const char *start_path);
bool  is_mach_file(const char *path);
char *get_base_filename(const char *path);

// helper
bool is_sep(char c);

#endif
