/*
 * Copyright 2019-2020,2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_gt9271.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief GT9271 I2C address. */
#define GT9271_I2C_ADDRESS0 (0x5D)
#define GT9271_I2C_ADDRESS1 (0x14)

#define GT9271_REG_ADDR_SIZE 2

/*! @brief GT9271 registers. */
#define GT9271_REG_ID             0x8140U
#define GT9271_CONFIG_ADDR        0x8047U
#define GT9271_REG_XL             0x8048U
#define GT9271_REG_XH             0x8049U
#define GT9271_REG_YL             0x804AU
#define GT9271_REG_YH             0x804BU
#define GT9271_REG_TOUCH_NUM      0x804CU
#define GT9271_REG_CONFIG_VERSION 0x8047U
#define GT9271_REG_MODULE_SWITCH1 0x804DU
#define GT9271_REG_STAT           0x814EU
#define GT9271_REG_FIRST_POINT    0x814FU

#define GT9271_STAT_BUF_MASK          (1U << 7U)
#define GT9271_STAT_POINT_NUMBER_MASK (0xFU << 0U)
#define GT9271_MODULE_SWITCH_X2Y_MASK (1U << 3U)
#define GT9271_MODULE_SWITCH_INT_MASK (3U << 0U)

#define GT9271_CONFIG_SIZE (186U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Verify firmware, return true if pass. */
static bool GT9271_VerifyFirmware(const uint8_t *firmware);
static uint8_t GT9271_GetFirmwareCheckSum(const uint8_t *firmware);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint8_t GT9271_GetFirmwareCheckSum(const uint8_t *firmware)
{
    uint8_t sum = 0;
    uint16_t i  = 0;

    for (i = 0; i < GT9271_CONFIG_SIZE - 2U; i++)
    {
        sum += (*firmware);
        firmware++;
    }

    return (~sum + 1U);
}

static bool GT9271_VerifyFirmware(const uint8_t *firmware)
{
    return ((firmware[GT9271_REG_CONFIG_VERSION - GT9271_CONFIG_ADDR] != 0U) &&
            (GT9271_GetFirmwareCheckSum(firmware) == firmware[GT9271_CONFIG_SIZE - 2U]));
}

status_t GT9271_Init(gt9271_handle_t *handle, const gt9271_config_t *config)
{
    status_t status;
    uint32_t deviceID;
    uint8_t gt9271Config[GT9271_CONFIG_SIZE];

    assert(NULL != handle);

    (void)memset(handle, 0, sizeof(*handle));

    handle->I2C_SendFunc     = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc  = config->I2C_ReceiveFunc;
    handle->timeDelayMsFunc  = config->timeDelayMsFunc;
    handle->pullResetPinFunc = config->pullResetPinFunc;

    /* Reset the panel and set the I2C address mode. */
    if (NULL != config->intPinFunc)
    {
        config->intPinFunc(kGT9271_IntPinPullDown);
    }
    if (NULL != config->pullResetPinFunc)
    {
        config->pullResetPinFunc(false);
    }

    /* >= 10ms. */
    handle->timeDelayMsFunc(20);

    if (kGT9271_I2cAddrAny == config->i2cAddrMode)
    {
        if (NULL != config->pullResetPinFunc)
        {
            config->pullResetPinFunc(true);
        }

        /* >= 55ms */
        handle->timeDelayMsFunc(55);

        if (NULL != config->intPinFunc)
        {
            config->intPinFunc(kGT9271_IntPinInput);
        }

        /* Try address 0 */
        handle->i2cAddr = GT9271_I2C_ADDRESS0;
        status = handle->I2C_ReceiveFunc(handle->i2cAddr, GT9271_REG_ID, GT9271_REG_ADDR_SIZE, (uint8_t *)&deviceID, 4);

        if (kStatus_Success != status)
        {
            /* Try address 1 */
            handle->i2cAddr = GT9271_I2C_ADDRESS1;
            status =
                handle->I2C_ReceiveFunc(handle->i2cAddr, GT9271_REG_ID, GT9271_REG_ADDR_SIZE, (uint8_t *)&deviceID, 4);

            if (kStatus_Success != status)
            {
                return status;
            }
        }
    }
    else
    {
        if (kGT9271_I2cAddrMode1 == config->i2cAddrMode)
        {
            if (NULL != config->intPinFunc)
            {
                config->intPinFunc(kGT9271_IntPinPullUp);
            }
            handle->i2cAddr = GT9271_I2C_ADDRESS1;
        }
        else
        {
            handle->i2cAddr = GT9271_I2C_ADDRESS0;
        }

        /* >= 100us */
        handle->timeDelayMsFunc(1);

        if (NULL != config->pullResetPinFunc)
        {
            config->pullResetPinFunc(true);
        }

        /* >= 5ms */
        handle->timeDelayMsFunc(5);

        if (NULL != config->intPinFunc)
        {
            config->intPinFunc(kGT9271_IntPinPullDown);
        }

        /* >= 50ms */
        handle->timeDelayMsFunc(50);

        if (NULL != config->intPinFunc)
        {
            config->intPinFunc(kGT9271_IntPinInput);
        }

        status = handle->I2C_ReceiveFunc(handle->i2cAddr, GT9271_REG_ID, GT9271_REG_ADDR_SIZE, (uint8_t *)&deviceID, 4);
        if (kStatus_Success != status)
        {
            return status;
        }
    }

    /* Verify the device. */
    if (deviceID != 0x31373239U)
    {
        return kStatus_Fail;
    }

    /* Initialize the IC. */
    status = handle->I2C_ReceiveFunc(handle->i2cAddr, GT9271_CONFIG_ADDR, GT9271_REG_ADDR_SIZE, gt9271Config,
                                     GT9271_CONFIG_SIZE);
    if (kStatus_Success != status)
    {
        return status;
    }

    /*
     * GT9271 driver gets the original firmware from touch panel control IC, modify
     * the configuration, then set it to the IC again. The original firmware
     * read from the touch IC must be correct, otherwise setting wrong firmware
     * to the touch IC will break it.
     */
    if (true != GT9271_VerifyFirmware(gt9271Config))
    {
        return kStatus_Fail;
    }

    handle->resolutionX = ((uint16_t)gt9271Config[GT9271_REG_XH - GT9271_CONFIG_ADDR]) << 8U;
    handle->resolutionX += gt9271Config[GT9271_REG_XL - GT9271_CONFIG_ADDR];
    handle->resolutionY = ((uint16_t)gt9271Config[GT9271_REG_YH - GT9271_CONFIG_ADDR]) << 8U;
    handle->resolutionY += gt9271Config[GT9271_REG_YL - GT9271_CONFIG_ADDR];

    /* Only update the firmware if necessary. */
    if ((gt9271Config[GT9271_REG_TOUCH_NUM - GT9271_CONFIG_ADDR] & 0x0FU) != (config->touchPointNum & 0x0FU) ||
        (gt9271Config[GT9271_REG_MODULE_SWITCH1 - GT9271_CONFIG_ADDR] & GT9271_MODULE_SWITCH_INT_MASK) !=
            (uint8_t)(config->intTrigMode))
    {
        gt9271Config[GT9271_REG_TOUCH_NUM - GT9271_CONFIG_ADDR] = (config->touchPointNum) & 0x0FU;
        gt9271Config[GT9271_REG_MODULE_SWITCH1 - GT9271_CONFIG_ADDR] &= (uint8_t)(~GT9271_MODULE_SWITCH_INT_MASK);
        gt9271Config[GT9271_REG_MODULE_SWITCH1 - GT9271_CONFIG_ADDR] |= (uint8_t)(config->intTrigMode);

        gt9271Config[GT9271_CONFIG_SIZE - 2U] = GT9271_GetFirmwareCheckSum(gt9271Config);
        gt9271Config[GT9271_CONFIG_SIZE - 1U] = 1U; /* Mark the firmware as valid. */

        status = handle->I2C_SendFunc(handle->i2cAddr, GT9271_CONFIG_ADDR, GT9271_REG_ADDR_SIZE, gt9271Config,
                                      GT9271_CONFIG_SIZE);
    }

    return status;
}

status_t GT9271_Deinit(gt9271_handle_t *handle)
{
    handle->pullResetPinFunc(false);
    return kStatus_Success;
}

static status_t GT9271_ReadRawTouchData(gt9271_handle_t *handle, uint8_t *touchPointNum)
{
    status_t status;
    uint8_t gt9271Stat;

    status = handle->I2C_ReceiveFunc(handle->i2cAddr, GT9271_REG_STAT, GT9271_REG_ADDR_SIZE, &gt9271Stat, 1);
    if (kStatus_Success != status)
    {
        *touchPointNum = 0;
        return status;
    }

    *touchPointNum = gt9271Stat & GT9271_STAT_POINT_NUMBER_MASK;

    if (0U != (gt9271Stat & GT9271_STAT_BUF_MASK))
    {
        if (*touchPointNum > 0U)
        {
            status = handle->I2C_ReceiveFunc(handle->i2cAddr, GT9271_REG_FIRST_POINT, GT9271_REG_ADDR_SIZE,
                                             (void *)handle->pointReg, (*touchPointNum) * sizeof(gt9271_point_reg_t));
        }

        /* Must set the status register to 0 after read. */
        gt9271Stat = 0;
        status    = handle->I2C_SendFunc(handle->i2cAddr, GT9271_REG_STAT, GT9271_REG_ADDR_SIZE, &gt9271Stat, 1);
    }
    else
    {
        status = kStatus_TOUCHPANEL_NotReady;
    }

    return status;
}

status_t GT9271_GetSingleTouch(gt9271_handle_t *handle, int *touch_x, int *touch_y)
{
    status_t status;
    uint8_t touchPointNum;

    status = GT9271_ReadRawTouchData(handle, &touchPointNum);

    if (kStatus_Success == status)
    {
        if (touchPointNum > 0U)
        {
            *touch_x =
                (int)(uint16_t)((uint16_t)handle->pointReg[0].lowX + (((uint16_t)handle->pointReg[0].highX) << 8U));
            *touch_y =
                (int)(uint16_t)((uint16_t)handle->pointReg[0].lowY + (((uint16_t)handle->pointReg[0].highY) << 8U));
        }
        else
        {
            status = (status_t)kStatus_TOUCHPANEL_NotTouched;
        }
    }
    else
    {
        status = kStatus_Fail;
    }

    return status;
}

status_t GT9271_GetMultiTouch(gt9271_handle_t *handle, uint8_t *touch_count, touch_point_t touch_array[])
{
    status_t status;
    uint32_t i;
    uint8_t desiredTouchPointNum;
    uint8_t actualTouchPointNum;

    status = GT9271_ReadRawTouchData(handle, &actualTouchPointNum);

    if (kStatus_Success == status)
    {
        desiredTouchPointNum = *touch_count;

        if (0U == actualTouchPointNum)
        {
            status = (status_t)kStatus_TOUCHPANEL_NotTouched;
        }
        else if (actualTouchPointNum > desiredTouchPointNum)
        {
            actualTouchPointNum = desiredTouchPointNum;
        }
        else
        {
            /* MISRA compatible. */
        }

        for (i = 0; i < actualTouchPointNum; i++)
        {
            touch_array[i].valid   = true;
            touch_array[i].touchID = handle->pointReg[i].id;
            touch_array[i].x       = handle->pointReg[i].lowX + (((uint16_t)handle->pointReg[i].highX) << 8U);
            touch_array[i].y       = handle->pointReg[i].lowY + (((uint16_t)handle->pointReg[i].highY) << 8U);
        }

        for (; i < desiredTouchPointNum; i++)
        {
            touch_array[i].valid = false;
        }
    }
    else
    {
        status = kStatus_Fail;
    }

    *touch_count = actualTouchPointNum;

    return status;
}

status_t GT9271_GetResolution(gt9271_handle_t *handle, int *resolutionX, int *resolutionY)
{
    *resolutionX = (int)handle->resolutionX;
    *resolutionY = (int)handle->resolutionY;

    return kStatus_Success;
}
