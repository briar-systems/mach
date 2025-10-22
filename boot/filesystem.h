#ifndef FILESYSTEM_H
#define FILESYSTEM_H

// read entire file into memory (binary safe)
char *fs_read_file(const char *path);

// check if file exists
int fs_file_exists(const char *path);

// recursively create directory and all parent directories
int fs_ensure_dir_recursive(const char *path);

// find project root by searching for mach.toml
char *fs_find_project_root(const char *start_path);

// get base filename without extension
char *fs_get_base_filename(const char *path);

// duplicate directory portion of path
char *fs_dirname(const char *path);

#endif
