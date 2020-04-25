#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

char *strdup2(const char *s) {
    if(!s)
        return NULL;
    char *n = malloc(strlen(s) + 1);
    if(n) {
        strcpy(n, s);
    }
    return n;
}

char *strstrr(const char *haystack, const char *needle) {
    const char *last = haystack - 1;
    while(1) {
        const char *new_last = strstr(last + 1, needle);
        if(new_last)
            last = new_last;
        else
            break;
    }
    if(last == haystack - 1)
        return NULL;
    else
        return (char *)last;
}

char *strrep(const char *haystack, const char *needle,
        const char *replacement) {
    size_t h_size = strlen(haystack);
    size_t n_size = strlen(needle);
    size_t rep_size = strlen(replacement);
    assert(h_size > 0);

    char *needle_loc = strstr(haystack, needle);
    if(!needle_loc)
        return strdup2(haystack);
    size_t prefix = needle_loc - haystack;

    // need to create a string:
    // [before needle] + [replacement] + [after needle]
    char *ret = malloc(1 + h_size - n_size + rep_size);
    strncpy(ret, haystack, prefix);
    strcpy(ret + prefix, replacement);
    strcpy(ret + prefix + rep_size, needle_loc + n_size);

    return ret;
}

static const char * const NULL_STR = "(null)";

char *concat_strs_arr(size_t n, char **strs) {
    size_t str_len = 1;
    size_t lens[n];

    for(size_t i = 0; i < n; ++i) {
        const char * const str = strs[i];
        if (str != NULL) {
            lens[i] = strlen(str);
        } else {
            lens[i] = strlen(NULL_STR);
        }
        str_len += lens[i];
    }

    char *s = malloc(str_len);
    s[0] = '\0';

    str_len = 0;
    for(size_t i = 0; i < n; ++i) {
        const char * const str = strs[i];
        if (str != NULL) {
            strcpy(s + str_len, str);
        } else {
            strcpy(s + str_len, NULL_STR);
        }
        str_len += lens[i];
    }

    return s;
}

char *concat_strs(size_t n, ...) {
    if(n == 0)
        return strdup2("");
    va_list args;
    char *strs[n];

    va_start(args, n);
    for(size_t i = 0; i < n; ++i) {
        strs[i] = va_arg(args, char *);
    }
    va_end(args);

    return concat_strs_arr(n, strs);
}

int is_slash(char c) {
    return (c == '/' || c == '\\');
}

char *remove_prefix(char *str, size_t n) {
    if(n == 0)
        return str;
    for(char *from = str + n, *to = str;; ++from, ++to) {
        *to = *from;
        if(*from == '\0')
            break;
    }
    return str;
}

char *lower_str(char *str) {
    for(char *c = str; *c; ++c) {
        if(*c >= 'A' && *c <= 'Z')
            *c = *c - 'A' + 'a';
    }
    return str;
}
