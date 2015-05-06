#include <stdlib.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "utils.h"

/**
 * @brief an opaque structure to hold the attributes of a file
 */
struct attrib_s {
#ifdef _WIN32
    DWORD attrib;
#endif
    char NOT_EMPTY;
};

char *advance_to_newline(char *s) {
    while(s[0] != '\0' && s[0] != '\n')
        s++;
    return s;
}

char *file_to_str(FILE *f) {
    int len = 0;
    rewind(f);
    while(getc(f) != EOF)
        len++;
    rewind(f);

    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';

    return buf;
}

#ifdef _WIN32
attrib_t *attrib_open(char *file) {
    attrib_t *attrib = malloc(sizeof(attrib_t));
    attrib->attrib = GetFileAttributes(file);
    SetFileAttributes(file, FILE_ATTRIBUTE_NORMAL);
    return attrib;
}
void attrib_save(char *file, attrib_t *attrib) {
    SetFileAttributes(file, attrib->attrib);
    free(attrib);
}
#else
attrib_t *attrib_open(char *file) {
    return NULL;
}
void attrib_save(char *file, attrib_t *attrib) {
    return;
}
#endif
