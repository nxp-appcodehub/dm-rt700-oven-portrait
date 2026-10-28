/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/


/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include "custom.h"


#include "ui_Oven.h"


uint32_t OvenTime = 25;

void oven_update_time (uint32_t time, OVEN_Time_T state)
{
	if(time)
	{
		OvenTime = time;
	}
	else
	{
		switch (state)
		{
		case kOVEN_TimeDown:
			OvenTime--;
			break;
		case kOVEN_TimeUp:
			OvenTime++;
			break;
		}
	}
	lv_label_set_text_fmt(guider_ui.screen.mins_num_label, "%02d",OvenTime);
	
	
}

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */

void custom_init(gg_ui_t *ui)
{
    /* Add your codes here */
	ui_oven_init();

	ui_oven_set_Temperature(165);

}


