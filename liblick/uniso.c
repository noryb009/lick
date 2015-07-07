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

int filter_file(const char *f) {
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

    return 1;
}

void find_if_special(uniso_status_t *s, const char *f, const char *dst) {
    // check for kernel or initrd
    if(s->kernel == NULL && (strstr(f, "vmlinu") || strstr(f, "VMLINU"))) {
        s->kernel = concat_strs(3, dst, "/", f);
    }
    if(s->initrd == NULL && (strstr(f, "initr") || strstr(f, "INITR"))) {
        s->initrd = concat_strs(3, dst, "/", f);
    }
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

uniso_progress_t count_in_iso(uniso_status_t *s, struct archive *iso) {
    uniso_progress_t total = 0;
    struct archive_entry *e;

    while(archive_read_next_header(iso, &e) == ARCHIVE_OK) {
        char *name = strdup2(archive_entry_pathname(e));
        if(archive_entry_filetype(e) != AE_IFDIR
                && filter_file(name))
            ++total;
        free(name);
    }

    return total;
}

int extract_iso(uniso_status_t *s, struct archive *iso, const char *dst,
        uniso_progress_t total, uniso_progress_cb cb, void *cb_data) {
    struct archive_entry *e;
    uniso_progress_t current = 0;

    make_dir_parents(dst);

    if(cb)
        cb(current, total, cb_data);

    while(archive_read_next_header(iso, &e) == ARCHIVE_OK) {
        char *name = strdup2(archive_entry_pathname(e));
        if(archive_entry_filetype(e) == AE_IFDIR
                || !filter_file(name)) {
            free(name);
            continue;
        }
        char *dest = create_dest(dst, "/", name);
        if(!extract_file(s, iso, dest)) {
            free(dest);
            free(name);
            return 0;
        }
        ++current;
        s->files = new_node(name, s->files);
        find_if_special(s, name, dst);
        if(cb)
            cb(current, total, cb_data);
        free(dest);
    }

    return 1;
}

struct archive *uniso_open(uniso_status_t *s, const char *src) {
    struct archive *iso = archive_read_new();
    archive_read_support_format_iso9660(iso);
    if(archive_read_open_filename(iso, src, 10240) != ARCHIVE_OK) {
        s->error = strdup2("Could not open ISO file.");
        return NULL;
    }
    return iso;
}

uniso_status_t *uniso(const char *src, const char *dst,
        uniso_progress_cb cb, void *cb_data) {
    uniso_status_t *s = new_status();

    struct archive *iso = uniso_open(s, src);
    if(!iso)
        return s;
    uniso_progress_t total = count_in_iso(s, iso);
    archive_read_free(iso);
    if(total == 0)
        return s;

    iso = uniso_open(s, src);
    if(!iso)
        return s;
    if(extract_iso(s, iso, dst, total, cb, cb_data))
        s->finished = 1;
    archive_read_free(iso);

    return s;
}

void free_uniso_status(uniso_status_t *s) {
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
