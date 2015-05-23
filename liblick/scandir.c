#include <stdlib.h>
#include <string.h>

#include "scandir.h"

int alphasort2(const struct dirent **a, const struct dirent **b) {
    return strcmp((*a)->d_name, (*b)->d_name);
}

int antialphasort2(const struct dirent **a, const struct dirent **b) {
    return alphasort2(b, a);
}

#define original_size 2

int scandir_main(const char *path, struct dirent ***e,
        int (*filter)(const void *),
        int (*compare)(const struct dirent **, const struct dirent **),
        int full_path) {
    int path_len;
    if(full_path == 1)
        path_len = strlen(path);
    DIR *d = opendir(path);
    if(d == NULL)
        return -1;

    int n = 0;
    int size = original_size;
    struct dirent **ents = malloc(sizeof(struct dirent *) * size);
    struct dirent *next;

    while(1) {
        next = readdir(d);
        if(next == NULL)
            break;

        if(strcmp(next->d_name, ".") == 0 || strcmp(next->d_name, "..") == 0)
            continue;
        if(filter != NULL)
            if(full_path == 0 && !filter(next))
                continue;
            else if(full_path == 1) {
                int len = path_len + 1 + strlen(next->d_name) + 1;
                char buf[len];
                strcat(strcat(strcpy(buf, path), "/"), next->d_name);
                if(!filter(buf))
                    continue;
            }

        if(size == n) {
            size *= 2;
            ents = realloc(ents, sizeof(struct dirent *) * size);
        }

        ents[n] = malloc(sizeof(struct dirent));
        ents[n]->d_ino = next->d_ino;
        strcpy(ents[n]->d_name, next->d_name);
        n++;
    }

    if(compare != NULL)
        qsort(ents, n, sizeof(struct dirent *),
                (int (*)(const void *, const void *))compare);

    closedir(d);
    *e = ents;
    return n;
}

int scandir2(const char *path, struct dirent ***e,
        int (*filter)(const struct dirent *),
        int (*compare)(const struct dirent **, const struct dirent **)) {
    scandir_main(path, e, (int (*)(const void *))filter, compare, 0);
}

int scandir_full_path(const char *path, struct dirent ***e,
        int (*filter)(const char *),
        int (*compare)(const struct dirent **, const struct dirent **)) {
    scandir_main(path, e, (int (*)(const void *))filter, compare, 1);
}
