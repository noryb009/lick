#pragma once

#include <stdio.h>

int makeDir(const char *d);
int makeDirR(const char *d);
int unlinkDir(const char *d);
int unlinkFile(const char *f);
char *strdup(const char *s);
char *dirname(const char *p);
char *basename(const char *p);
char *concat_strs(int n, ...);
char *file_to_str(FILE *f);
char *advance_to_newline(char *s);
int file_exists(char *path);
char *read_line(FILE *f, int *done);
