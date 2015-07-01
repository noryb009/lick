#include <archive.h>
#include <archive_entry.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uniso.h"
#include "utils.h"

uniso_status_t *new_status() {
    uniso_status_t *s = malloc(sizeof(uniso_status_t));
    s->finished = 0;
    s->files = NULL;
    s->initrd = NULL;
    s->kernel = NULL;
    s->error = NULL;
    return s;
}

int filter_file(uniso_status_t *s, const char *f, const char *dst) {
    if(strcmp(f, "boot.cat") == 0
            || strcmp(f, "boot.msg") == 0
            || strcmp(f, "help.msg") == 0
            || strcmp(f, "help2.msg") == 0
            || strcmp(f, "isolinux.bin") == 0
            || strcmp(f, "isolinux.cfg") == 0
            || strcmp(f, "logo.16") == 0
            || strcmp(f, "README.HTM") == 0
            || strstr(f, "/") != NULL) {
        return 0;
    }

    // check for kernel or initrd
    if(s->kernel == NULL && (strstr(f, "vmlinu") || strstr(f, "VMLINU"))) {
        s->kernel = concat_strs(3, dst, "/", f);
    }
    if(s->initrd == NULL && (strstr(f, "initr") || strstr(f, "INITR"))) {
        s->initrd = concat_strs(3, dst, "/", f);
    }

    return 1;
}

char *create_dest(const char *dst, const char *path, const char *f) {
    char *s = malloc(strlen(dst) + strlen(path) + strlen(f) + 1);
    s = strcat(strcat(strcpy(s, dst), path), f);
    return s;
}

int extract_file(uniso_status_t *s, struct archive *iso, const char *dst) {
    FILE *out = fopen(dst, "wb");
    if(!out) {
        s->error = strdup2("Error opening output file.");
        return 0;
    }

    char buf[1024];
    int size;

    for(;;) {
        size = archive_read_data(iso, buf, 1024);
        if(size == 0) {
            break;
        } else if(size < 0) {
            fclose(out);
            return 0;
        } else {
            fwrite(buf, 1, size, out);
        }
    }
    fclose(out);
    return 1;
}

int extract_iso(uniso_status_t *s, struct archive *iso, const char *dst) {
    struct archive_entry *e;

    make_dir_parents(dst);

    while(archive_read_next_header(iso, &e) == ARCHIVE_OK) {
        const struct stat *stat = archive_entry_stat(e);
        char *name = strdup2(archive_entry_pathname(e));
        if(S_ISDIR(stat->st_mode) || !filter_file(s, name, dst)) {
            free(name);
            continue;
        }
        s->files = new_node(name, s->files);

        char *dest = create_dest(dst, "/", name);
        if(!extract_file(s, iso, dest)) {
            free(dest);
            return 0;
        }
        free(dest);
    }

    return 1;
}

uniso_status_t *uniso(const char *src, const char *dst) {
    uniso_status_t *s = new_status();

    struct archive *iso = archive_read_new();
    archive_read_support_format_iso9660(iso);
    if(archive_read_open_filename(iso, src, 10240) != ARCHIVE_OK) {
        s->error = strdup2("Could not open ISO file.");
        return s;
    }

    if(extract_iso(s, iso, dst)) {
        s->finished = 1;
    }

    archive_read_free(iso);
    return s;
}

void uniso_status_free(uniso_status_t *s) {
    if(s->initrd != NULL) {
        free(s->initrd);
    }
    if(s->kernel != NULL) {
        free(s->kernel);
    }
    if(s->error != NULL) {
        free(s->error);
    }
    free_list(s->files, free);
    free(s);
}
