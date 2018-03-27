/**
 * @file
 * @brief Puppy Linux support
 */

#pragma once

#include "distro.h"

#define DISTRO_PUPPY_KEY "puppy"

distro_t *get_distro_puppy();

// used within Fatdog
distro_info_node_t *distro_puppy(string_node_t *files, const char *dst, const char *name, lickdir_t *lick);
int distro_puppy_filter(const char *f);
