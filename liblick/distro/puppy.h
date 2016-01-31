/**
 * @file
 * @brief Puppy Linux support
 */

#pragma once

#include "../distro.h"

distro_t *get_distro_puppy();

// used within Fatdog
node_t *distro_puppy(node_t *files, const char *dst, const char *name);
int distro_puppy_filter(const char *f);
