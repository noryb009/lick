#pragma once

#include "lickdir.h"

/**
 * @brief determine if the given ID is valid
 * @param id the ID to check
 * @param lick the lick directory
 * @param install_path the drive or path to install under
 * @return 1 if the ID is valid; 0 if it is not valid
 */
int is_valid_id(char *id, lickdir_t *lick, char *install_path);
/**
 * @brief generate a valid ID from an ISO path
 *
 * Given an ISO path name, takes the base name of the ISO without the .iso
 * extension, then converts any invalid characters to dashes. If this ID
 * is used, appends a number, from 2 to 100. If a valid ID has still not
 * been found, returns NULL.
 *
 * @param iso the ISO path
 * @param lick the lick directory
 * @param install_path the drive or path to install under
 * @return an ID, or NULL if a valid ID could not be created
 */
char *gen_id(char *iso, lickdir_t *lick, char *install_path);
/**
 * @brief generate a human-readable name from an ISO path
 *
 * Given an ISO path name, takes the base name of the ISO without the .iso
 * extension, then converts any dashes and underscores to spaces.
 *
 * @param iso the ISO path
 * @return a human-readable name
 */
char *gen_name(char *iso);
