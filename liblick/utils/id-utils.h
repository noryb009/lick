/**
 * @file
 * @brief Handle and generate names and IDs
 */

#pragma once

#include "lickdir.h"

/**
path@brief determine if the given ID is valid
 * @param id the ID to check
 * @param lick the lick directory
 * @param install_path the drive or path to install under
 * @return 1 if the ID is valid; 0 if it is not valid
 */
int is_valid_id(const char *id, lickdir_t *lick, const char *install_path);
/**
 * @brief generate a valid ID from a path
 *
 * Given a path name, suggests a nice id.
 *
 * @param path the path
 * @param lick the lick directory
 * @param install_path the drive or path to install under
 * @return an ID, or NULL if a valid ID could not be created
 */
char *gen_id(const char *path, lickdir_t *lick, const char *install_path);
/**
 * @brief generate a human-readable name from a path
 *
 * Given a path, suggests a nice name.
 *
 * @param path the path
 * @return a human-readable name
 */
char *gen_name(const char *path);
