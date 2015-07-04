#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "utils.h"
#include "../drives.h"
#include "../utils.h"

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
    if(s == NULL)
        return NULL;
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
    if(fread(buf, 1, len, f) < len) {
        free(buf);
        return NULL;
    }
    buf[len] = '\0';

    return buf;
}

char *file_to_str_no_rewind(FILE *f) {
    int len = 512;
    int cur = 0;
    char *buf = malloc(len);

    while(1) {
        int read = fread(buf + cur, 1, len - cur, f);
        if(read < len - cur) {
            buf[cur + read] = '\0';
            return buf;
        }
        cur += read;
        len *= 2;
        buf = realloc(buf, len);
    }
}

#ifdef _WIN32
attrib_t *attrib_get(const char *file) {
    attrib_t *attrib = malloc(sizeof(attrib_t));
    attrib->attrib = GetFileAttributes(file);
    return attrib;
}

attrib_t *attrib_open(const char *file) {
    attrib_t *attrib = attrib_get(file);
    SetFileAttributes(file, FILE_ATTRIBUTE_NORMAL);
    return attrib;
}
void attrib_save(const char *file, attrib_t *attrib) {
    SetFileAttributes(file, attrib->attrib);
    free(attrib);
}
#else
attrib_t *attrib_get(const char *file) {
    return NULL;
}
attrib_t *attrib_open(const char *file) {
    return NULL;
}
void attrib_save(const char *file, attrib_t *attrib) {
    return;
}
#endif
int backup_file(const char *f) {
    int len = strlen(f) + strlen(".bak") + 1;
    char buf[len];
    strcat(strcpy(buf, f), ".bak");
    if(!copy_file(buf, f)) {
        return 0;
    }

    attrib_save(buf, attrib_get(f));
    return 1;
}

int get_id_from_command_range(const char *c, char *out, char *start, char *end) {
    out[0] = '\0';

    char *buf;
    if(!run_system_output(c, &buf)) {
        free(buf);
        return 0;
    }

    char *start_loc = buf;
    if(start != NULL) {
        char *end_loc = strstr(buf, end);
        if(end_loc == NULL) {
            free(buf);
            return 0;
        }
        end_loc[0] = '\0';
        start_loc = strstrr(buf, start);
        if(start_loc == NULL) {
            free(buf);
            return 0;
        }
    }
    char *id = strchr(start_loc, '{');
    if(!id) {
        free(buf);
        return 0;
    }
    char *id_end = strchr(id, '}');
    if(!id_end) {
        free(buf);
        return 0;
    }
    id_end[0] = '\0';
    if(strlen(id + 1) > ID_LEN - 1) {
        free(buf);
        return 0;
    }
    strcpy(out, id + 1);
    free(buf);
    return 1;
}

int get_id_from_command(const char *c, char *out) {
    return get_id_from_command_range(c, out, NULL, NULL);
}

char *get_bcdedit() {
    char *path = get_windows_path();
    char *edit[] = {"/System32/bcdedit.exe", "/Sysnative/bcdedit.exe", NULL};
    char *c = malloc(COMMAND_BUFFER_LEN);

    for(int i = 0; edit[i] != NULL; ++i) {
        snprintf(c, COMMAND_BUFFER_LEN, "%s%s", path, edit[i]);
        if(path_exists(c)) {
            free(path);
            return c;
        }
    }

    free(path);
    free(c);
    return NULL;
}
