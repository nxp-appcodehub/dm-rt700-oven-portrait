/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Flash configuration variant selector.
 *
 * This wrapper compiles exactly one of the board-specific flash_config
 * sources based on the USE_FRDM_FLASH_CONFIG macro:
 *   - USE_FRDM_FLASH_CONFIG undefined or 0 : MIMXRT700-EVK (flash_config/evk)
 *   - USE_FRDM_FLASH_CONFIG == 1           : FRDM-RT700    (flash_config/frdm)
 *
 * The actual per-board files under evk/ and frdm/ are left untouched.
 * Define USE_FRDM_FLASH_CONFIG=1 (for example from CMake) to pick FRDM.
 */

/* Set to 1 for FRDM-RT700, 0 (or comment out) for MIMXRT700-EVK. */
#define USE_FRDM_FLASH_CONFIG 0

#if defined(USE_FRDM_FLASH_CONFIG) && (USE_FRDM_FLASH_CONFIG == 1)

#include "frdm/flash_config.c"
#else
#include "evk/flash_config.c"
#endif
