#include <stdlib.h>
#include <string.h>

#include "edit-flat-menu.h"
#include "../utils.h"

typedef struct {
    enum {
        T_LINE,
        T_DOUBLE_COMMENT,
    } type;
    const char *start;
    size_t len;
} token_t;

typedef struct {
    int start;
    type_e type;
    char *id;
} comment_token_t;

int is_double_comment(const char *c) {
    c += 2;
    while(*c == ' ' || *c == '\t')
       ++c;
    if(*c == '#' || *c == '\n')
        return 0;
    return 1;
}

/**
 * @brief gets a token from a file, ending in "##" or a newline
 *
 * Contains all letters of a line including a newline; all letters up to ## but
 * not including the ##; or all letters up to a '\0' but not including the null
 *
 * @param[in] f the current location in the file
 * @param[out] t the token to put info into
 */
void get_double_comment(const char *f, token_t *t) {
    t->type = T_LINE;
    t->start = f;

    const char *c = f;
    // while not done
    for(; *c != '\0' && *c != '\n'; ++c) {
        // if double comment
        if(*c == '#' && c[1] == '#' && is_double_comment(c)) {
            t->type = T_DOUBLE_COMMENT;
            --c;
            break;
        // if has content
        } else if(*c != ' ' && *c != '\t') {
            // continue to end of line
            while(*c != '\0' && *c != '\n')
                 ++c;
            // if NULL, back up to it
            if(*c == '\0')
                --c;
            break;
        }
    }
    t->len = c - f + 1;
}

int starts_with(const char **c, const char *tok) {
    // move to next non-whitespace token
    while(**c == ' ' || **c == '\t')
       ++(*c);

    int ret = 0;
    size_t len = strlen(tok);
    // if starts with token, then has a space, tab, newline, or end of string
    if(!strncasecmp(tok, *c, len) && strchr(" \t\n", (*c)[len])) {
        *c += len;
        ret = 1;
    }

    // move to next non-whitespace token
    while(**c == ' ' || **c == '\t')
       ++(*c);

    return ret;
}

/**
 * @brief extract info from a double comment line in a menu string
 *
 * Given a string such as "## start section abc" or "## end footer", fills the
 * given comment_token_t with relevent info
 *
 * @param c[in] the beginning of the double comment. Must begin with "##" and be a valid double comment
 * @param t[out] the token to fill info into
 * @return a pointer to the start of the next line
 */
const char *read_double_comment(const char *c, comment_token_t *t) {
    c += 2;

    // rest can be:
    //      start section abc
    //      start header
    //      start abc
    //      start
    //      end section abc
    //      end section
    //      end abc
    //      end

    // get if start or end
    if(starts_with(&c, "start"))
        t->start = 1;
    else if(starts_with(&c, "end"))
        t->start = 0;
    else
        t->start = 1;

    // get if header, section or footer
    if(starts_with(&c, "header"))
        t->type = S_HEADER;
    else if(starts_with(&c, "section"))
        t->type = S_SECTION;
    else if(starts_with(&c, "footer"))
        t->type = S_FOOTER;
    else
        t->type = S_SECTION;

    // get next newline, which marks end of id
    const char *c_newline = c;
    while(*c_newline != '\n' && *c_newline != '\0')
       ++c_newline;

    // rest of line is ID
    size_t id_len = c_newline - c;
    if(id_len == 0)
        t->id = NULL;
    else {
        t->id = malloc(id_len + 1);
        strncpy(t->id, c, id_len);
        t->id[id_len] = '\0';
    }

    if(*c_newline == '\0')
        return c_newline;
    else
        return c_newline + 1;
}

section_t *reset_section(section_t *s, const char *c) {
    s->type = S_UNLABELED;
    s->id = NULL;
    s->content = c;
    s->content_len = 0;
    return s;
}

section_t *new_part_section(const char *c) {
    return reset_section(malloc(sizeof(section_t)), c);
}

section_t *new_section(type_e type, const char *id, const char *content) {
    section_t *s = malloc(sizeof(section_t));
    s->type = type;
    s->id = strdup2(id);
    s->content = content;
    s->content_len = strlen(content);
    return s;
}

void free_section(section_t *s) {
    if(s->id)
        free(s->id);
    free(s);
}

int is_valuable_section(section_t *s) {
    if(s->type != S_UNLABELED)
        return 1;
    for(size_t c = 0; c < s->content_len; ++c) {
        if(s->content[c] != '\n' && s->content[c] != ' ' && s->content[c] != '\t')
            return 1;
    }
    return 0;
}

section_node_t *get_sections(const char *f) {
    section_node_t *ret = NULL;
    section_t *cur = new_part_section(f);
    token_t t;
    comment_token_t tc;

    const char *c = f;
    // while not done reading file
    while(*c != '\0') {
        // read a token
        get_double_comment(c, &t);
        if(t.type == T_DOUBLE_COMMENT) {
            c = read_double_comment(c + t.len, &tc);
            if(tc.start) {
                if(is_valuable_section(cur)) {
                    ret = new_section_node_t(cur, ret);
                    cur = new_part_section(c);
                } else
                    reset_section(cur, c);
                cur->id = tc.id;
                tc.id = NULL;
                cur->type = tc.type;
            } else {
                if(!cur->type)
                    cur->type = tc.type;
                if(!cur->id) {
                    cur->id = tc.id;
                    tc.id = NULL;
                }
                ret = new_section_node_t(cur, ret);
                cur = new_part_section(c);
            }
        } else {
            cur->content_len += t.len;
            c += t.len;
        }
    }

    if(is_valuable_section(cur))
        ret = new_section_node_t(cur, ret);
    else
        free(cur);

    // remove ending newline on unlabeled sections
    for(section_node_t *n = ret; n != NULL; n = n->next) {
        cur = n->val;
        if(cur->type == S_UNLABELED && cur->content_len > 0
                && cur->content[cur->content_len-1] == '\n')
            --cur->content_len;
    }

    return section_node_t_reverse(ret);
}

void write_section_type(FILE *f, section_t *sec) {
    switch(sec->type) {
    case S_SECTION:
        fprintf(f, "section");
        if(sec->id)
            fprintf(f, " %s", sec->id);
        break;
    case S_HEADER:
        fprintf(f, "header");
        break;
    case S_FOOTER:
        fprintf(f, "footer");
        break;
    case S_UNLABELED:
        break;
    }
}

void write_section(FILE *f, section_t *sec, int first) {
    if(sec->type != S_UNLABELED) {
        if(!first)
            fprintf(f, "\n");

        fprintf(f, "## start ");
        write_section_type(f, sec);
        fprintf(f, "\n");
    }

    fprintf(f, "%.*s", sec->content_len, sec->content);
    if(sec->content_len > 0 && sec->content[sec->content_len-1] != '\n')
        fprintf(f, "\n");

    if(sec->type != S_UNLABELED) {
        fprintf(f, "## end ");
        write_section_type(f, sec);
        fprintf(f, "\n");
    }
}

void write_sections(FILE *f, section_node_t *secs) {
    int first = 1;
    for(section_node_t *n = secs; n != NULL; n = n->next) {
        write_section(f, n->val, first);
        first = 0;
    }
}

section_node_t *append_section(section_node_t *secs, section_t *sec) {
    section_node_t *prev = NULL;

    for(section_node_t *n = secs;; n = n->next) {
        if(n == NULL || sec->type == S_HEADER
                || (((section_t *)n->val)->type == S_FOOTER
                    && sec->type != S_FOOTER)) {
            section_node_t *node = new_section_node_t(sec, n);
            if(!prev)
                return node;
            prev->next = node;
            return secs;
        }
        prev = n;
    }
}

section_node_t *remove_section(section_node_t *secs, const char *sec) {
    section_node_t *prev = NULL;

    for(section_node_t *n = secs; n != NULL; n = n->next) {
        section_t *cur = n->val;
        if(cur->id && !strcmp(cur->id, sec)) {
            if(prev)
                prev->next = n->next;
            else
                secs = n->next;
            free_section(cur);
            free(n);
            break;
        }
        prev = n;
    }

    return secs;
}

int flat_append_section(const char *menu_path, const char *id, const char *section,
        lickdir_t *lick) {
    char *menu = NULL;
    section_node_t *secs = NULL;

    if(path_exists(menu_path)) {
        FILE *f = fopen(menu_path, "r");
        if(!f) {
            if(!lick->err)
                lick->err = strdup2("Could not read menu!");
            return 0;
        }
        menu = file_to_str(f);
        fclose(f);
        secs = get_sections(menu);
    }

    section_t *s = new_section(S_SECTION, id, section);
    secs = append_section(secs, s);

    FILE *f = fopen(menu_path, "w");
    if(!f) {
        free_section_node_t(secs);
        if(menu)
            free(menu);
        if(!lick->err)
            lick->err = strdup2("Could not write to menu!");
        return 0;
    }
    write_sections(f, secs);
    fclose(f);
    free_section_node_t(secs);
    if(menu)
        free(menu);
    return 1;
}


int flat_remove_section(const char *menu_path, const char *id,
        lickdir_t *lick) {
    FILE *f = fopen(menu_path, "r");
    if(!f)
        return 1;
    char *menu = file_to_str(f);
    fclose(f);

    section_node_t *secs = get_sections(menu);
    secs = remove_section(secs, id);

    f = fopen(menu_path, "w");
    if(!f) {
        if(!lick->err)
            lick->err = strdup2("Could not write to menu!");
        free_section_node_t(secs);
        free(menu);
        return 0;
    }
    write_sections(f, secs);
    fclose(f);
    free_section_node_t(secs);
    free(menu);
    return 1;
}
