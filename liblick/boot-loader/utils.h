/**
 * @file
 * @brief functions used by multiple boot loader plugins
 */
#pragma once

#include <stdio.h>

#define ID_LEN 37
#define COMMAND_BUFFER_LEN 1024

struct attrib_s;
typedef struct attrib_s attrib_t;

/**
 * @brief returns a pointer to the next newline in a string
 * @param s the beginning of the string to search in
 * @return a pointer to the first newline, or to the string terminator
 */
char *advance_to_newline(char *s);
/**
 * @brief read a file into a string
 * @param f the file to read
 * @return the contents of the file, in a string
 */
char *file_to_str(FILE *f);
/**
 * @brief get the attributes of a file, and set the file attributes for writing
 * @param file the file
 * @return the old attributes of the file
 */
attrib_t *attrib_open(const char *file);
/**
 * @brief restore the attributes of a file
 * @param file the file
 * @param attrib the old attributes
 */
void attrib_save(const char *file, attrib_t *attrib);
/**
 * @brief create a backup of a file
 * @param f the file to backup
 * @return 1 on success
 */
int backup_file(const char *f);
/**
 * @brief get an ID from a command, between two strings
 *
 * Looks for end in the output, then looks backwards to find the start, then
 * looks for the ID between these two points.
 *
 * @param c the command
 * @param out the buffer to put the ID. Must be at least ID_LEN + 1 long
 * @param start the string to look after
 * @param end the string to look before
 * @return 1 on success, otherwise 0
 */
int get_id_from_command_range(const char *c, char *out, char *start, char *end);
/**
 * @brief get an ID from a command
 * @param c the command
 * @param out the buffer to put the ID. Must be at least ID_LEN + 1 long
 * @return 1 on success, otherwise 0
 */
int get_id_from_command(const char *c, char *out);
/**
 * @brief get the path to bcdedit.exe
 * @return path to bcdedit.exe. Must be freed using free
 */
char *get_bcdedit();
