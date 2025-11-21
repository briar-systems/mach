#ifndef IOUTIL_H
#define IOUTIL_H

#include <stdbool.h>

bool  is_directory(char *path);
bool  file_exists(char *path);
bool  path_is_absolute(char *path);
char *path_dirname(char *path);
char *path_lastname(char *path);
char *path_join(char *a, char *b);
char *path_relative(char *base, char *path);
char *path_get_extension(char *path);

char *read_file(char *path);

char **list_files(char *path);
char **list_files_recursive(char *path, char **file_list, int count);

#endif
