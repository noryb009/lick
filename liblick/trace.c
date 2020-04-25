#include "trace.h"

#include <stdio.h>
#include <stdlib.h>

struct trace_state_t {
    /// The file.
    FILE *f;
};

trace_state_t *create_trace_state(const char *log_file_path) {
    trace_state_t *state = malloc(sizeof(trace_state_t));
    state->f = NULL;

    state->f = fopen(log_file_path, "a");
    if (!state->f) {
        free_trace_state(state);
        return NULL;
    }

    return state;
}

void free_trace_state(trace_state_t *state) {
    if (state == NULL) {
        return;
    }

    if (state->f != NULL) {
        fclose(state->f);
    }

    free(state);
}

void trace_str(trace_state_t *state, const char *str) {
    fprintf(state->f, "%s\n", str);
}
