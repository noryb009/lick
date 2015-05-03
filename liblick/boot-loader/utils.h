#pragma once

#include <stdio.h>

struct attrib_s;
typedef struct attrib_s attrib_t;

char *advance_to_newline(char *s);
char *file_to_str(FILE *f);
attrib_t *attrib_open(char *file);
void attrib_save(char *file, attrib_t *attrib);
