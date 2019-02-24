/**
 * @file
 * @brief Fat Dog support
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "distro.h"

#define DISTRO_FATDOG_KEY "fatdog"

distro_t *get_distro_fatdog();

#ifdef __cplusplus
}
#endif
