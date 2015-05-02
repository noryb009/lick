#pragma once

#include <stdio.h>

int makeDir(const char *d);
int makeDirR(const char *d);
int unlinkDir(const char *d);
int unlinkFile(const char *f);
char *strdup(const char *s);
char *concat_strs(int n, ...);
int is_file(char *path);
int file_exists(char *path);
char *read_line(FILE *f, int *done);
int is_conf_file(const char *name);
void conf_option(char *ln, char **keyword_start, char **item_start);
