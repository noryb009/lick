#include <archive.h>
#include <archive_entry.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scandir.h"
#include "uniso.h"
#include "utils.h"

uniso_status_t *new_status() {
    uniso_status_t *s = malloc(sizeof(uniso_status_t));
    s->finished = 0;
    s->files = NULL;
    s->error = NULL;
    return s;
}

char *create_dest(const char *dst, const char *path, const char *f) {
    char *s = malloc(strlen(dst) + strlen(path) + strlen(f) + 1);
    s = strcat(strcat(strcpy(s, dst), path), f);
    return s;
}

int extract_file(uniso_status_t *s, struct archive *iso, const char *dst) {
    FILE *out = fopen(dst, "wb");
    if(!out) {
        s->error = concat_strs(3, "Error opening output file `", dst, "'");
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

uniso_progress_t count_in_iso(struct archive *iso,
        distro_filter_f filter) {
    uniso_progress_t total = 0;
    struct archive_entry *e;

    while(archive_read_next_header(iso, &e) == ARCHIVE_OK) {
        char *name = strdup2(archive_entry_pathname(e));
        if(archive_entry_filetype(e) != AE_IFDIR
                && filter(name))
            ++total;
        free(name);
    }

    return total;
}

int extract_iso(uniso_status_t *s, struct archive *iso, const char *dst,
        distro_filter_f filter, uniso_progress_t total, uniso_progress_cb cb,
        void *cb_data) {
    struct archive_entry *e;
    uniso_progress_t current = 0;

    make_dir_parents(dst);

    if(cb)
        cb(current, total, cb_data);

    while(archive_read_next_header(iso, &e) == ARCHIVE_OK) {
        char *name = unix_path(strdup2(archive_entry_pathname(e)));
        if(archive_entry_filetype(e) == AE_IFDIR
                || !filter(name)) {
            free(name);
            continue;
        }
        s->files = new_string_node_t(name, s->files);
        char *dest = unix_path(create_dest(dst, "/", name));
        if(!extract_file(s, iso, dest)) {
            free(dest);
            return 0;
        }
        ++current;
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

uniso_status_t *undir(const char *src, const char *dst, distro_filter_f filter,
        uniso_progress_cb cb, void *cb_data) {
    uniso_status_t *s = new_status();

    make_dir_parents(dst);

    struct dirent **e;
    const unsigned int len = scandir2(src, &e, NULL, NULL);
    int failed = 0;

    if(cb)
        cb(0, len, cb_data);

    for(uniso_progress_t i = 0; i < len; /* Updated in loop. */) {
        if(!failed) {
            char *path = unix_path(concat_strs(3, src, "/", e[i]->d_name));
            if(file_type(path) != FILE_TYPE_DIR && filter(e[i]->d_name)) {
                char *dst_path = unix_path(concat_strs(3, dst, "/", e[i]->d_name));
                if(copy_file(dst_path, path)) {
                    s->files = new_string_node_t(strdup(e[i]->d_name), s->files);
                    // If we are copying directly from a CD, the file will be
                    // read-only. This causes problems when we try to delete
                    // the files.
                    attrib_open(dst_path);
                } else {
                    failed = 1;
                }
                free(dst_path);
            }
            free(path);
        }
        free(e[i]);
        ++i;
        if(cb)
            cb(i, len, cb_data);
    }
    free(e);

    s->finished = 1;
    return s;
}

uniso_status_t *uniso(const char *src, const char *dst, distro_filter_f filter,
        uniso_progress_cb cb, void *cb_data) {
    // Deal with directories.
    if(file_type(src) == FILE_TYPE_DIR) {
      return undir(src, dst, filter, cb, cb_data);
    }

    uniso_status_t *s = new_status();

    struct archive *iso = uniso_open(s, src);
    if(!iso)
        return s;
    uniso_progress_t total = count_in_iso(iso, filter);
    archive_read_free(iso);
    if(total == 0)
        return s;

    iso = uniso_open(s, src);
    if(!iso)
        return s;
    if(extract_iso(s, iso, dst, filter, total, cb, cb_data))
        s->finished = 1;
    archive_read_free(iso);

    return s;
}

void free_uniso_status(uniso_status_t *s) {
    if(s->error != NULL) {
        free(s->error);
    }
    free_string_node_t(s->files);
    free(s);
}
