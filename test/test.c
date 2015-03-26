#include <stdio.h>

#include "uniso.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    makeDirR("/tmp/b");
    uniso_status_t *s = uniso(argv[1], "/tmp/b");
    uniso_status_free(s);
}
