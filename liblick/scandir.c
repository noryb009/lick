#include <stdlib.h>
#include <string.h>

#include "scandir.h"

int alphasort2(const struct dirent **a, const struct dirent **b) {
    return strcmp((*b)->d_name, (*a)->d_name);
}

#define original_size 2

int scandir2(const char *path, struct dirent ***e,
        int (*filter)(const struct dirent *),
        int (*compare)(const struct dirent **, const struct dirent **)) {
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

        if(filter != NULL && !filter(next))
            continue;

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
