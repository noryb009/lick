#include "utils/id-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/fs-utils.h"
#include "utils/string-utils.h"

/**
 * Return the iso file name, without the `.iso` extension.
 */
static char *gen_name_base_iso(const char *iso) {
    char id_arr[strlen(iso) + 1];
    char *id = id_arr;
    strcpy(id, iso);
    while(1) {
        char *last_slash = strpbrk(id, "/\\");
        if(last_slash)
            id = last_slash + 1;
        else
            break;
    }

    char *iso_loc = strstr(id, ".iso");
    if(iso_loc)
        iso_loc[0] = '\0';

    return strdup2(id);
}

static char *gen_name_base_dir(const char *dir) {
    const char * const generic_name = "Linux";
    size_t len = strlen(dir);
    // Cut off any trailing slashes.
    while (len > 0 && (dir[len - 1] == '/' || dir[len - 1] == '\\')) {
        --len;
    }

    // We either have a directory or a drive.
    // Drives have no more slashes in them.
    size_t last_slash = len;
    for (size_t i = len; i > 0; /* In loop. */) {
        --i;
        if (dir[i] == '/' || dir[i] == '\\') {
            last_slash = i;
            break;
        }
    }
    if (last_slash == 0)
        return strdup2(generic_name);

    // If we have a drive, use a generic name.
    if (last_slash == len)
        // TODO: ask distros to guess a name.
        return strdup2(generic_name);
    else {
        // We want the text from last_slash + 1 to len, inclusive.
        const size_t copy_len = len - last_slash;
        char *ret = malloc((copy_len + 1) * sizeof(char));
        memcpy(ret, dir + last_slash + 1, copy_len);
        ret[copy_len] = '\0';
        return ret;
    }
}

char *gen_name_base(const char *path) {
    if (file_type(path) == FILE_TYPE_DIR) {
        return gen_name_base_dir(path);
    } else {
        return gen_name_base_iso(path);
    }
}

int is_valid_id_char(const char c) {
    if((c >= '0' && c <= '9')
            || (c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z'))
        return 1;
    switch(c) {
    case '-':
    case '.':
    case '_':
        return 1;
    }
    return 0;
}

int path_exists_free(char *path) {
    int ret = path_exists(path);
    free(path);
    return ret;
}

int is_valid_id(const char *id, lickdir_t *lick, const char *install_path) {
    for(int i = 0; id[i] != '\0'; ++i)
        if(!is_valid_id_char(id[i]))
            return 0;

    if(path_exists_free(concat_strs(3, install_path, "/", id)))
        return 0;

    if(path_exists_free(concat_strs(4, lick->entry, "/", id, ".conf")))
        return 0;

    if(path_exists_free(concat_strs(4, lick->entry, "/", id, ".conf")))
        return 0;

    return 1;
}

char *gen_id(const char *path, lickdir_t *lick, const char *install_path) {
    char *id = gen_name_base(path);

    // remove invalid chars
    for(int i = 0; id[i] != '\0'; ++i)
        if(!is_valid_id_char(id[i]))
            id[i] = '-';

    if(is_valid_id(id, lick, install_path)) {
        return id;
    }

    char *base_id = malloc(strlen(id) + 4 + 1);
    strcpy(base_id, id);

    for(int i = 2; i < 100; ++i) {
        sprintf(base_id, "%s-%d", id, i);
        if(is_valid_id(base_id, lick, install_path)) {
            free(id);
            return base_id;
        }
    }

    // unlikely to happen
    free(id);
    free(base_id);
    return NULL;
}

char *gen_name(const char *path) {
    char *name = gen_name_base(path);

    for(int i = 0; name[i] != '\0'; ++i) {
        char c = name[i];
        switch(c) {
        case '-':
        case '_':
            name[i] = ' ';
        }
    }

    return name;
}
