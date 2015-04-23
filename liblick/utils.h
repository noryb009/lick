#pragma once

#include <stdio.h>

int makeDir(const char *d);
int makeDirR(const char *d);
char *strdup(const char *s);
char *dirname(const char *p);
char *basename(const char *p);
char *concat_strs(int n, ...);
char *file_to_str(FILE *f);
char *get_windows_drive();
char *advance_to_newline(char *s);
