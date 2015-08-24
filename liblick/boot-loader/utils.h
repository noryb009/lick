/**
 * @file
 * @brief functions used by multiple boot loader plugins
 */
#pragma once

#include <stdio.h>

#include "../lickdir.h"

#define LOADER_DESC "LICK Boot Loader"
#define START_LOADER_DESC "Start LICK Boot Loader"

#define ID_LEN 37
#define COMMAND_BUFFER_LEN 1024

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define attrib_t DWORD
#else
#define attrib_t char
#endif

/**
 * @brief returns a pointer to the next newline in a string
 * @param s the beginning of the string to search in
 * @return a pointer to the first newline, or to the string terminator
 */
char *advance_to_newline(char *s);
/**
 * @brief find the extents of an INI section
 * @param[in] haystack the file
 * @param[in] needle the section header, including brackets
 * @param[out] start points to first character of section
 * @param[out] end points to first character of the next section, or '\0'
 * @return 1 if the section is found, otherwise 0
 */
int find_section(const char *haystack, const char *needle, char **start, char **end);
/**
 * @brief find the line after the last needle in an INI section
 *
 * sec and sec_end can be start and end from find_section
 *
 * @param sec the beginning of the section
 * @param sec_end the beginning of the next section
 * @param needle a needle to determine if something is an entry
 */
char *after_last_entry(char *sec, char *sec_end, const char *needle);
/**
 * @brief check the timeout value of an INI file
 *
 * f must be allocated using malloc, and should contain the contents of an
 * INI file. f will be freed in this function
 *
 * @param f the file
 * @param key the key of the timeout
 * @param sep the string before the start of the timeout
 * @return the INI file. Must be freed using free
 */
char *check_timeout(char *f, char *key, char *sep);
/**
 * @brief get the attributes of a file, and set the file attributes for writing
 * @param file the file
 * @return the old attributes of the file
 */
attrib_t attrib_open(const char *file);
/**
 * @brief restore the attributes of a file
 * @param file the file
 * @param attrib the old attributes
 */
void attrib_save(const char *file, attrib_t attrib);
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
/**
 * @brief applies a function to a file
 *
 * Reads a file and applies the function to the contents. If the return value
 * is a string, optionally backs up the file, then saves the return value in
 * the file. This also ensures the file is saved with the same attributes.
 *
 * @param file the file to apply the function on
 * @param fn the function to apply
 * @param backup to backup the file or not before saving
 * @param lick the lick environment
 * @return 1 on success
 */
int apply_fn_to_file(const char *file, char *(*fn)(char *, lickdir_t *),
        int backup, lickdir_t *lick);
