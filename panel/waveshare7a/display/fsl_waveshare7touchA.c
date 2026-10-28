/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Driver for the Waveshare 7-DSI-TOUCH-A display.
 * LCD controller: ILI9881C
 * Interface:      MIPI DSI 2-lane, DPI mode, 24-bit RGB888
 * Resolution:     720 x 1280
 *
 * Power-on sequence (I2C, device 0x45) derived from the Waveshare ESP32
 * reference driver (esp_lcd_ili9881c).  The ILI9881C register init table
 * is a direct conversion of vendor_specific_init_default[] from that same
 * reference.
 */

#include <string.h>
#include "fsl_display.h"
#include "fsl_waveshare7touchA.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define WS7TA_DelayMs VIDEO_DelayMs

/* ILI9881C page-select command and magic bytes */
#define ILI9881C_CMD_PAGE_SEL  (0xFFU)
#define ILI9881C_BYTE0         (0x98U)
#define ILI9881C_BYTE1         (0x81U)
#define ILI9881C_PAGE0         (0x00U)
#define ILI9881C_PAGE1         (0x01U)
#define ILI9881C_PAGE3         (0x03U)
#define ILI9881C_PAGE4         (0x04U)

/* Internal init-command descriptor */
typedef struct
{
    uint8_t  cmd;
    uint8_t  dataSize; /* number of data bytes; 0 means no data payload */
    uint16_t delayMs;
    uint8_t  data[4];
} ws7ta_init_cmd_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
const display_operations_t ws7ta_ops = {
    .init   = WS7TA_Init,
    .deinit = WS7TA_Deinit,
    .start  = WS7TA_Start,
    .stop   = WS7TA_Stop,
};

/*
 * Full ILI9881C init sequence for the Waveshare 7-DSI-TOUCH-A.
 * Converted from vendor_specific_init_default[] in esp_lcd_ili9881c.
 * {cmd, dataSize, delayMs, {data...}}
 */
static const ws7ta_init_cmd_t s_ws7taInitCmds[] = {

    /* --- CMD Page 3 --- */
    {ILI9881C_CMD_PAGE_SEL, 3, 0, {ILI9881C_BYTE0, ILI9881C_BYTE1, ILI9881C_PAGE3}},
    {0x01, 1, 0, {0x00}},
    {0x02, 1, 0, {0x00}},
    {0x03, 1, 0, {0x73}},
    {0x04, 1, 0, {0x00}},
    {0x05, 1, 0, {0x00}},
    {0x06, 1, 0, {0x0A}},
    {0x07, 1, 0, {0x00}},
    {0x08, 1, 0, {0x00}},
    {0x09, 1, 0, {0x61}},
    {0x0A, 1, 0, {0x00}},
    {0x0B, 1, 0, {0x00}},
    {0x0C, 1, 0, {0x01}},
    {0x0D, 1, 0, {0x00}},
    {0x0E, 1, 0, {0x00}},
    {0x0F, 1, 0, {0x61}},
    {0x10, 1, 0, {0x61}},
    {0x11, 1, 0, {0x00}},
    {0x12, 1, 0, {0x00}},
    {0x13, 1, 0, {0x00}},
    {0x14, 1, 0, {0x00}},
    {0x15, 1, 0, {0x00}},
    {0x16, 1, 0, {0x00}},
    {0x17, 1, 0, {0x00}},
    {0x18, 1, 0, {0x00}},
    {0x19, 1, 0, {0x00}},
    {0x1A, 1, 0, {0x00}},
    {0x1B, 1, 0, {0x00}},
    {0x1C, 1, 0, {0x00}},
    {0x1D, 1, 0, {0x00}},
    {0x1E, 1, 0, {0x40}},
    {0x1F, 1, 0, {0x80}},
    {0x20, 1, 0, {0x06}},
    {0x21, 1, 0, {0x01}},
    {0x22, 1, 0, {0x00}},
    {0x23, 1, 0, {0x00}},
    {0x24, 1, 0, {0x00}},
    {0x25, 1, 0, {0x00}},
    {0x26, 1, 0, {0x00}},
    {0x27, 1, 0, {0x00}},
    {0x28, 1, 0, {0x33}},
    {0x29, 1, 0, {0x03}},
    {0x2A, 1, 0, {0x00}},
    {0x2B, 1, 0, {0x00}},
    {0x2C, 1, 0, {0x00}},
    {0x2D, 1, 0, {0x00}},
    {0x2E, 1, 0, {0x00}},
    {0x2F, 1, 0, {0x00}},
    {0x30, 1, 0, {0x00}},
    {0x31, 1, 0, {0x00}},
    {0x32, 1, 0, {0x00}},
    {0x33, 1, 0, {0x00}},
    {0x34, 1, 0, {0x04}},
    {0x35, 1, 0, {0x00}},
    {0x36, 1, 0, {0x00}},
    {0x37, 1, 0, {0x00}},
    {0x38, 1, 0, {0x3C}},
    {0x39, 1, 0, {0x00}},
    {0x3A, 1, 0, {0x00}},
    {0x3B, 1, 0, {0x00}},
    {0x3C, 1, 0, {0x00}},
    {0x3D, 1, 0, {0x00}},
    {0x3E, 1, 0, {0x00}},
    {0x3F, 1, 0, {0x00}},
    {0x40, 1, 0, {0x00}},
    {0x41, 1, 0, {0x00}},
    {0x42, 1, 0, {0x00}},
    {0x43, 1, 0, {0x00}},
    {0x44, 1, 0, {0x00}},
    {0x50, 1, 0, {0x10}},
    {0x51, 1, 0, {0x32}},
    {0x52, 1, 0, {0x54}},
    {0x53, 1, 0, {0x76}},
    {0x54, 1, 0, {0x98}},
    {0x55, 1, 0, {0xBA}},
    {0x56, 1, 0, {0x10}},
    {0x57, 1, 0, {0x32}},
    {0x58, 1, 0, {0x54}},
    {0x59, 1, 0, {0x76}},
    {0x5A, 1, 0, {0x98}},
    {0x5B, 1, 0, {0xBA}},
    {0x5C, 1, 0, {0xDC}},
    {0x5D, 1, 0, {0xFE}},
    {0x5E, 1, 0, {0x00}},
    {0x5F, 1, 0, {0x0E}},
    {0x60, 1, 0, {0x0F}},
    {0x61, 1, 0, {0x0C}},
    {0x62, 1, 0, {0x0D}},
    {0x63, 1, 0, {0x06}},
    {0x64, 1, 0, {0x07}},
    {0x65, 1, 0, {0x02}},
    {0x66, 1, 0, {0x02}},
    {0x67, 1, 0, {0x02}},
    {0x68, 1, 0, {0x02}},
    {0x69, 1, 0, {0x01}},
    {0x6A, 1, 0, {0x00}},
    {0x6B, 1, 0, {0x02}},
    {0x6C, 1, 0, {0x15}},
    {0x6D, 1, 0, {0x14}},
    {0x6E, 1, 0, {0x02}},
    {0x6F, 1, 0, {0x02}},
    {0x70, 1, 0, {0x02}},
    {0x71, 1, 0, {0x02}},
    {0x72, 1, 0, {0x02}},
    {0x73, 1, 0, {0x02}},
    {0x74, 1, 0, {0x02}},
    {0x75, 1, 0, {0x0E}},
    {0x76, 1, 0, {0x0F}},
    {0x77, 1, 0, {0x0C}},
    {0x78, 1, 0, {0x0D}},
    {0x79, 1, 0, {0x06}},
    {0x7A, 1, 0, {0x07}},
    {0x7B, 1, 0, {0x02}},
    {0x7C, 1, 0, {0x02}},
    {0x7D, 1, 0, {0x02}},
    {0x7E, 1, 0, {0x02}},
    {0x7F, 1, 0, {0x01}},
    {0x80, 1, 0, {0x00}},
    {0x81, 1, 0, {0x02}},
    {0x82, 1, 0, {0x14}},
    {0x83, 1, 0, {0x15}},
    {0x84, 1, 0, {0x02}},
    {0x85, 1, 0, {0x02}},
    {0x86, 1, 0, {0x02}},
    {0x87, 1, 0, {0x02}},
    {0x88, 1, 0, {0x02}},
    {0x89, 1, 0, {0x02}},
    {0x8A, 1, 0, {0x02}},

    /* --- CMD Page 4 --- */
    {ILI9881C_CMD_PAGE_SEL, 3, 0, {ILI9881C_BYTE0, ILI9881C_BYTE1, ILI9881C_PAGE4}},
    {0x38, 1, 0, {0x01}},
    {0x39, 1, 0, {0x00}},
    {0x6C, 1, 0, {0x15}},
    {0x6E, 1, 0, {0x2A}},
    {0x6F, 1, 0, {0x33}},
    {0x3A, 1, 0, {0x94}},
    {0x8D, 1, 0, {0x14}},
    {0x87, 1, 0, {0xBA}},
    {0x26, 1, 0, {0x76}},
    {0xB2, 1, 0, {0xD1}},
    {0xB5, 1, 0, {0x06}},
    {0x3B, 1, 0, {0x98}},

    /* --- CMD Page 1 (gamma + power) --- */
    {ILI9881C_CMD_PAGE_SEL, 3, 0, {ILI9881C_BYTE0, ILI9881C_BYTE1, ILI9881C_PAGE1}},
    {0x22, 1, 0, {0x0A}},
    {0x31, 1, 0, {0x00}},
    {0x53, 1, 0, {0x71}},
    {0x55, 1, 0, {0x8F}},
    {0x40, 1, 0, {0x33}},
    {0x50, 1, 0, {0x96}},
    {0x51, 1, 0, {0x96}},
    {0x60, 1, 0, {0x23}},
    /* positive gamma */
    {0xA0, 1, 0, {0x08}},
    {0xA1, 1, 0, {0x1D}},
    {0xA2, 1, 0, {0x2A}},
    {0xA3, 1, 0, {0x10}},
    {0xA4, 1, 0, {0x15}},
    {0xA5, 1, 0, {0x28}},
    {0xA6, 1, 0, {0x1C}},
    {0xA7, 1, 0, {0x1D}},
    {0xA8, 1, 0, {0x7E}},
    {0xA9, 1, 0, {0x1D}},
    {0xAA, 1, 0, {0x29}},
    {0xAB, 1, 0, {0x6B}},
    {0xAC, 1, 0, {0x1A}},
    {0xAD, 1, 0, {0x18}},
    {0xAE, 1, 0, {0x4B}},
    {0xAF, 1, 0, {0x20}},
    {0xB0, 1, 0, {0x27}},
    {0xB1, 1, 0, {0x50}},
    {0xB2, 1, 0, {0x64}},
    {0xB3, 1, 0, {0x39}},
    /* negative gamma */
    {0xC0, 1, 0, {0x08}},
    {0xC1, 1, 0, {0x1D}},
    {0xC2, 1, 0, {0x2A}},
    {0xC3, 1, 0, {0x10}},
    {0xC4, 1, 0, {0x15}},
    {0xC5, 1, 0, {0x28}},
    {0xC6, 1, 0, {0x1C}},
    {0xC7, 1, 0, {0x1D}},
    {0xC8, 1, 0, {0x7E}},
    {0xC9, 1, 0, {0x1D}},
    {0xCA, 1, 0, {0x29}},
    {0xCB, 1, 0, {0x6B}},
    {0xCC, 1, 0, {0x1A}},
    {0xCD, 1, 0, {0x18}},
    {0xCE, 1, 0, {0x4B}},
    {0xCF, 1, 0, {0x20}},
    {0xD0, 1, 0, {0x27}},
    {0xD1, 1, 0, {0x50}},
    {0xD2, 1, 0, {0x64}},
    {0xD3, 1, 0, {0x39}},

    /* --- CMD Page 0 (main registers + sleep-out + display-on) --- */
    {ILI9881C_CMD_PAGE_SEL, 3, 0, {ILI9881C_BYTE0, ILI9881C_BYTE1, ILI9881C_PAGE0}},
    {0x3A, 1, 0,   {0x77}}, /* pixel format RGB888 (24 bpp) */
    {0x36, 1, 0,   {0x00}}, /* MADCTL - no flip */
    {0x35, 1, 0,   {0x00}}, /* tearing effect off */
    {0x11, 0, 150, {0x00}}, /* Sleep Out - wait 150 ms */
    {0x29, 0,  20, {0x00}}, /* Display On - wait 20 ms */
};

/*******************************************************************************
 * Private functions
 ******************************************************************************/

/*
 * Send one ILI9881C command via MIPI DSI Generic Write.
 * The first byte of the DSI payload is always the register address (cmd);
 * it is followed by the optional data bytes.
 */
static status_t WS7TA_SendCommand(mipi_dsi_device_t *dsiDevice,
                                  const ws7ta_init_cmd_t *entry)
{
    uint8_t buf[8];

    buf[0] = entry->cmd;

    if (entry->dataSize > 0U)
    {
        (void)memcpy(&buf[1], entry->data, (size_t)entry->dataSize);
    }

    return MIPI_DSI_GenericWrite(dsiDevice, buf, (int32_t)(entry->dataSize + 1U));
}

/*
 * Execute the I2C power-on sequence required by the Waveshare 7-DSI-TOUCH-A
 * before the DSI panel initialisation can begin.
 *
 * Sequence derived from the Waveshare ESP32 reference driver:
 *   esp_lcd_ili9881c / esp_lcd_new_panel_ili9881c()
 */
static status_t WS7TA_PowerEnable(const ws7ta_resource_t *resource)
{
    status_t status;

    status = resource->writeRegister(0x95U, 0x11U);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = resource->writeRegister(0x95U, 0x17U);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = resource->writeRegister(0x96U, 0x00U);
    if (status != kStatus_Success)
    {
        return status;
    }

    WS7TA_DelayMs(100U);

    status = resource->writeRegister(0x96U, 0xFFU);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Allow the panel to stabilise before sending DSI traffic */
    WS7TA_DelayMs(1000U);

    return kStatus_Success;
}

/*******************************************************************************
 * Public API
 ******************************************************************************/

status_t WS7TA_Init(display_handle_t *handle, const display_config_t *config)
{
    status_t status;
    const ws7ta_resource_t *resource = (const ws7ta_resource_t *)(handle->resource);
    mipi_dsi_device_t *dsiDevice     = resource->dsiDevice;
    uint32_t i;

    /* Verify resolution */
    if (config->resolution != FSL_VIDEO_RESOLUTION(WS7TA_PANEL_WIDTH, WS7TA_PANEL_HEIGHT))
    {
        return kStatus_InvalidArgument;
    }

    /* Step 1: power on the panel controller via I2C */
    status = WS7TA_PowerEnable(resource);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Step 2: send the full ILI9881C register init sequence over DSI */
    for (i = 0U; i < ARRAY_SIZE(s_ws7taInitCmds); i++)
    {
        status = WS7TA_SendCommand(dsiDevice, &s_ws7taInitCmds[i]);
        if (status != kStatus_Success)
        {
            return status;
        }

        if (s_ws7taInitCmds[i].delayMs != 0U)
        {
            WS7TA_DelayMs((uint32_t)s_ws7taInitCmds[i].delayMs);
        }
    }

    return kStatus_Success;
}

status_t WS7TA_Deinit(display_handle_t *handle)
{
    return WS7TA_Stop(handle);
}

status_t WS7TA_Start(display_handle_t *handle)
{
    const ws7ta_resource_t *resource = (const ws7ta_resource_t *)(handle->resource);
    mipi_dsi_device_t *dsiDevice     = resource->dsiDevice;

    return MIPI_DSI_DCS_SetDisplayOn(dsiDevice, true);
}

status_t WS7TA_Stop(display_handle_t *handle)
{
    const ws7ta_resource_t *resource = (const ws7ta_resource_t *)(handle->resource);
    mipi_dsi_device_t *dsiDevice     = resource->dsiDevice;

    return MIPI_DSI_DCS_SetDisplayOn(dsiDevice, false);
}
