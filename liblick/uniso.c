#include <cdio/cdio.h>
#include <cdio/iso9660.h>
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

int extract_file(uniso_status_t *s, iso9660_t *iso, iso9660_stat_t *stat,
        const char *dst) {
    printf("1\n");
    char dst_name[strlen(dst) + 1];
    FILE *out = fopen(unix_path(strcpy(dst_name, dst)), "wb");
    printf("2\n");
    if(!out) {
        s->error = strdup2("Error opening output file.");
        return 0;
    }

    // size in blocks, rounded up
    unsigned int blks = stat->size / ISO_BLOCKSIZE;
    if(stat->size % ISO_BLOCKSIZE != 0) ++blks;
    printf("3\n");

    char buf[ISO_BLOCKSIZE];

    printf("4\n");
    for(unsigned int i = 0, total = 0; i < blks; ++i, total += ISO_BLOCKSIZE) {
    printf("5\n");
        lsn_t lsn = stat->lsn + i;
        memset(buf, 0, ISO_BLOCKSIZE);

    printf("6\n");
        if(ISO_BLOCKSIZE != iso9660_iso_seek_read(iso, buf, lsn, 1)) {
            fclose(out);
            return 0;
        }

    printf("7\n");
        if(stat->size - total >= ISO_BLOCKSIZE)
            fwrite(buf, ISO_BLOCKSIZE, 1, out);
        else
            fwrite(buf, stat->size - total, 1, out);

    printf("8\n");
        if(ferror(out))
            return 0;
    }
    printf("9\n");

    fclose(out);
    return 1;
}

uniso_progress_t count_in_iso(uniso_status_t *s, CdioList_t *files) {
    uniso_progress_t total = 0;
    CdioListNode_t *file;

    _CDIO_LIST_FOREACH(file, files) {
        char name[4096];
        iso9660_stat_t *stat = (iso9660_stat_t *)_cdio_list_node_data(file);
        iso9660_name_translate(stat->filename, name);
        if(stat->type != _STAT_DIR
                && filter_file(name))
            ++total;
    }

    return total;
}

int extract_iso(uniso_status_t *s, iso9660_t *iso, const char *dst,
        uniso_progress_cb cb, void *cb_data) {
    CdioList_t *files = iso9660_ifs_readdir(iso, "/");
    if(!files)
        return 0;

    uniso_progress_t total = count_in_iso(s, files);
    uniso_progress_t current = 0;

    make_dir_parents(dst);

    if(cb)
        cb(current, total, cb_data);

    CdioListNode_t *file;
    _CDIO_LIST_FOREACH(file, files) {
        char name[4096];
        iso9660_stat_t *stat = (iso9660_stat_t *)_cdio_list_node_data(file);
        iso9660_name_translate(stat->filename, name);
        if(stat->type != _STAT_DIR
                && filter_file(name)) {
            char *dest = create_dest(dst, "/", name);
            if(!extract_file(s, iso, stat, dest)) {
                free(dest);
                return 0;
            }
            ++current;
            s->files = new_node(strdup2(name), s->files);
            find_if_special(s, name, dst);
            if(cb)
                cb(current, total, cb_data);
            free(dest);
        }
    }

    return 1;
}

iso9660_t *uniso_open(uniso_status_t *s, const char *src) {
    printf("z\n");
    iso9660_t *iso = iso9660_open(src);
    printf("y\n");
    if(iso == NULL) {
        s->error = strdup2("Could not open ISO file.");
        return NULL;
    }
    return iso;
}

uniso_status_t *uniso(const char *src, const char *dst,
        uniso_progress_cb cb, void *cb_data) {
    printf("a\n");
    uniso_status_t *s = new_status();

    printf("bb, %s\n", src);
    iso9660_t *iso = uniso_open(s, src);
    if(!iso)
        return s;
    printf("c\n");

    if(extract_iso(s, iso, dst, cb, cb_data))
        s->finished = 1;
    printf("d\n");

    iso9660_close(iso);
    printf("DONE\n");

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
