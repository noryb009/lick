#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "utils.h"
#include "../drives.h"
#include "../utils.h"

char *advance_to_newline(char *s) {
    if(s == NULL)
        return NULL;
    while(s[0] != '\0' && s[0] != '\n')
        s++;
    return s;
}

char *advance_to_nextline(char *s) {
    if(s == NULL)
        return NULL;
    char *newline = advance_to_newline(s);
    if(newline[0] == '\n')
        ++newline;
    return newline;
}

int find_section(const char *haystack, const char *needle, char **start, char **end) {
    *start = strstr(haystack, needle);
    if(*start == NULL) {
        *end = NULL;
        return 0;
    }
    *end = strchr(*start + 1, '[');
    if(*end == NULL)
        *end = strchr(*start, '\0');
    return 1;
}

char *after_last_entry(char *sec, char *sec_end, const char *needle) {
    // isolate section
    char old_end = sec_end[0];
    sec_end[0]= '\0';
    // find line after last menuitem=
    char *item = strstrr(sec, needle);
    sec_end[0] = old_end;

    if(item != NULL)
        // found last item, move to next line
        item = advance_to_newline(item);
    else
        // otherwise, put right after start of section
        item = advance_to_newline(sec);

    if(item[0] == '\n') {
        // item is pointing to the newline after the last entry.
        // replace the '\n' with `\nentry=...', injecting the entry after
        //   the current last entry
        item[0] = '\0';
        return item + 1;
    } else
        // item is pointing to the end of the file.
        // print the entire file, then `\nentry=...'
        return item;
}

char *check_timeout(char *f, char *key, char *sep) {
    char *loc = strstr(f, key);
    if(!loc) // did not find key, use system default
        return f;

    size_t s = strlen(sep);
    while(strncmp(loc, sep, s) != 0) {
        // did not find key separator, add separator
        if(*loc == '\n' || *loc == '\0') {
            *loc = '\0';
            char *after = loc;
            if(*loc == '\n')
                after = loc + 1;

            char *ret = concat_strs(4, f, sep, "5\n", after);
            free(f);
            return ret;
        }
        ++loc;
    }

    // get timeout
    loc += strlen(sep);
    while(*loc < '0' || *loc > '9') {
        // if non-numeric value, just return
        if(*loc == '\n' || *loc == '\0')
            return f;
        ++loc;
    }

    int val = 0;
    char *num_end;
    for(num_end = loc; *num_end >= '0' && *num_end <= '9'; ++num_end)
        if(val < 100) // avoid overflow, anything bigger doesn't matter anyway
            val = val * 10 + (*num_end - '0');

    // if acceptable timeout, return original
    if(val >= 3 && val <= 30)
        return f;

    // loc at first number digit, num_end at character after last digit
    *loc = '\0';
    char *ret = concat_strs(3, f, "5", num_end);
    free(f);
    return ret;
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
attrib_t attrib_get(const char *file) {
    return GetFileAttributes(file);
}

attrib_t attrib_open(const char *file) {
    attrib_t attrib = attrib_get(file);
    SetFileAttributes(file, FILE_ATTRIBUTE_NORMAL);
    return attrib;
}
void attrib_save(const char *file, attrib_t attrib) {
    SetFileAttributes(file, attrib);
}
#else
attrib_t attrib_get(const char *file) {
    return 0;
}
attrib_t attrib_open(const char *file) {
    return 0;
}
void attrib_save(const char *file, attrib_t attrib) {
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

int apply_fn_to_file(const char *file, char *(*fn)(char *, lickdir_t *),
        int backup, lickdir_t *lick) {
    FILE *f = fopen(file, "r");
    if(!f) {
        if(!lick->err)
            lick->err = concat_strs(2, "Could not load boot loader file: ", file);
        return 0;
    }
    char *contents = file_to_str(f);
    fclose(f);

    char *new_contents = fn(contents, lick);
    free(contents);
    if(!new_contents) {
        if(!lick->err)
            lick->err = strdup2("Could not modify boot loader file.");
        return 0;
    }

    if(backup)
        backup_file(file);

    attrib_t attrib = attrib_open(file);
    f = fopen(file, "w");
    if(!f) {
        attrib_save(file, attrib);
        free(new_contents);
        if(!lick->err)
            lick->err = concat_strs(2, "Could not open boot loader file for writing", file);
        return 0;
    }

    fprintf(f, "%s", new_contents);
    fclose(f);
    free(new_contents);
    attrib_save(file, attrib);

    return 1;
}
