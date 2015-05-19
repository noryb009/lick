#include <stdlib.h>
#include <string.h>

#include "id-utils.h"
#include "utils.h"

char *gen_name_base(char *iso) {
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
    iso_loc[0] = '\0';

    return strdup(id);
}

int is_valid_id_char(char c) {
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

int is_valid_id(char *id, lickdir_t *lick, char *install_path) {
    for(int i = 0; id[i] != '\0'; ++i)
        if(!is_valid_id_char(id[i]))
            return 0;

    char *check = concat_strs(3, install_path, "/", id);

    if(path_exists_free(concat_strs(4, lick->entry, "/", id, ".conf")))
        return 0;

    if(path_exists_free(concat_strs(4, lick->entry, "/", id, ".conf")))
        return 0;

    if(path_exists_free(concat_strs(4, lick->menu, "/50-", id, ".conf")))
        return 0;

    return 1;
}

char *gen_id(char *iso, lickdir_t *lick, char *install_path) {
    char *id = gen_name_base(iso);

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

char *gen_name(char *iso) {
    char *name = gen_name_base(iso);

    int j = 0;
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
