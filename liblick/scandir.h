#pragma once

#include <dirent.h>

int alphasort2(const struct dirent **a, const struct dirent **b);
int scandir2(const char *path, struct dirent ***e,
        int (*filter)(const struct dirent *),
        int (*compare)(const struct dirent **, const struct dirent **));
