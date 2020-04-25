/**
 * @file
 * @brief A simple logging library.
 */

#pragma once

struct trace_state_t;

/**
 * @brief A wrapper around the tracing state.
 */
typedef struct trace_state_t trace_state_t;

/**
 * @brief Create a new trace state. Must be freed using `free_trace_state`.
 * 
 * @param log_file_path Desired path of the log file. Bad things might happen
 *    if this is opened by another process.
 */
trace_state_t *create_trace_state(const char *log_file_path);

/**
 * @brief Free the given trace state.
 */
void free_trace_state(trace_state_t *state);

/**
 * @brief Trace the given line.
 */
void trace_str(trace_state_t *state, const char *str);