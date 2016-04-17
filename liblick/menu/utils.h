/**
 * @file
 * @brief Functions used in multiple menus
 */

#pragma once

#define INDENT "    "

/**
 * @brief Determines if a file has valuable info
 *
 * Determines if a path to a file has valuable info.
 * In this case, any non-header and non-footer section is considered not-valuable.
 * @param menu A path to a flag menu file
 * @return 1 if the file has valuable info, 0 otherwise
 */
int has_valuable_info(const char *menu);
