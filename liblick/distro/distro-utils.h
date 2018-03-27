/**
 * @file
 * @brief Functions used by multiple distros
 */

#pragma once

#include "distro.h"

distro_info_t *new_distro_info(char *k, char *i, char *op, char *ft);
distro_info_t *new_empty_distro_info();
distro_info_t *copy_distro_info(distro_info_t *s);
distro_t *new_distro(distro_e id, const char *key, const char *name,
        distro_info_f info, distro_filter_f filter);
