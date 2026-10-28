/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_WAVESHARE7TOUCHA_H_
#define _FSL_WAVESHARE7TOUCHA_H_

#include "fsl_display.h"
#include "fsl_mipi_dsi_cmd.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * Waveshare 7-DSI-TOUCH-A ribbon cable (22-pin FPC) - I2C pins:
 *   Pin  1 - 3V3  (power)
 *   Pin  2 - SDA  (I2C data)
 *   Pin  3 - SCL  (I2C clock)
 *   ...
 *   Pin 22 - (last pin)
 * I2C device address: 0x45
 */
#define WS7TA_CTRL_ADDR (0x45U)

/* ILI9881C panel resolution */
#define WS7TA_PANEL_WIDTH  (720U)
#define WS7TA_PANEL_HEIGHT (1280U)

/*!
 * @brief Waveshare 7" Touch A (ILI9881C) display resource.
 *
 * Mirrors the existing wsdisp_resource_t layout so the same board I2C
 * helpers can be reused without modification.
 */
typedef struct _ws7ta_resource
{
    mipi_dsi_device_t *dsiDevice;                              /*!< MIPI DSI device. */
    status_t (*readStatus)(uint8_t regAddr, uint8_t *value);   /*!< Read a register via I2C. */
    status_t (*writeRegister)(uint8_t regAddr, uint8_t value); /*!< Write a register via I2C. */
} ws7ta_resource_t;

extern const display_operations_t ws7ta_ops;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

status_t WS7TA_Init(display_handle_t *handle, const display_config_t *config);

status_t WS7TA_Deinit(display_handle_t *handle);

status_t WS7TA_Start(display_handle_t *handle);

status_t WS7TA_Stop(display_handle_t *handle);

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_WAVESHARE7TOUCHA_H_ */
