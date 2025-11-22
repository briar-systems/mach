#ifndef FILESYSTEM_H
#define FILESYSTEM_H

// join varargs of paths into a single path
char *fs_path_join(const char *first, ...);

// change current working directory
int fs_chdir(const char *path);

// read entire file into memory (binary safe)
char *fs_read_file(const char *path);

// check if file exists
int fs_file_exists(const char *path);

// check if path is a directory
int fs_is_directory(const char *path);

// check if path ends with .mach extension
int fs_is_mach_file(const char *path);

// recursively create directory and all parent directories
int fs_ensure_dir_recursive(const char *path);

// find project root by searching for mach.toml
char *fs_find_project_root(const char *start_path);

// get base filename without extension
char *fs_get_base_filename(const char *path);

// duplicate directory portion of path
char *fs_dirname(const char *path);

// recursively list all .mach files in directory
// returns array of absolute paths, terminated by NULL
char **fs_list_mach_files_recursive(const char *dir_path);

// free array of strings
void fs_free_string_array(char **array);

#endif
