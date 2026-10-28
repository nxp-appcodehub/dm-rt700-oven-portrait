/*
 * Copyright 2022-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef UI_OVEN_H_
#define UI_OVEN_H_

#include "gui_guider.h"

LV_FONT_DECLARE(lv_font_Antonio_Regular_50)
LV_FONT_DECLARE(lv_font_Antonio_Regular_22)
LV_FONT_DECLARE(lv_font_Antonio_Regular_20)
LV_FONT_DECLARE(lv_font_Antonio_Regular_30)

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FADE_ANIM_TIME			300

typedef enum
{
	kOVEN_ModeFFRE,
	kOVEN_ModeULEC,
	kOVEN_ModeFFULC,
	kOVEN_ModeFFLEC,
	kOVEN_ModeGrill,
	kOVEN_ModeFFG,
	kOVEN_ModeUOE,
	kOVEN_ModeLOE,
	kOVEN_ModeDeFrost,
	kOVEN_ModeNull
} OVEN_MODE_T;

typedef enum
{
	kOVEN_ModeDirLeft,
	kOVEN_ModeDirRight,
} OVEN_MODE_Dir_T;

typedef enum
{
	kOVEN_DoorClose,
	kOVEN_DoorOpen,
} OVEN_Door_T;

typedef enum
{
	kOVEN_lightOff,
	kOVEN_LightOn,
} OVEN_Light_T;

typedef enum
{
	kOVEN_Start,
	kOVEN_Stop,
} OVEN_State_T;

typedef enum
{
	kOVEN_FanOff,
	kOVEN_FanOn,
} OVEN_Fan_T;

typedef enum
{
	kVIT_Start = 1,
	kVIT_Stop,
	kVIT_OpenDoor,
	kVIT_CloseDoor,
	kVIT_LightOn,
	kVIT_LightOff,
	kVIT_SetTemp150,
	kVIT_SetTemp180,
	kVIT_SetTemp210,
	kVIT_DefrotsMode,
	kVIT_ModeToLeft,
	kVIT_ModeToRight
} VIT_Oven_Commands_T;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ui_oven_init (void);

void ui_oven_set_Temperature(uint32_t temp);

/* Play the oven door image sequence forward (closed -> open). */
void ui_oven_door_open_anim(void);

/* Play the open animation but start on a specific frame
   (0 = closed dark, 1 = closed with light on). */
void ui_oven_door_open_anim_from(uint8_t start_frame);

/* Play the oven door image sequence backward (open -> closed). */
void ui_oven_door_close_anim(void);

/* Play the close animation but stop on a specific end frame
   (0 = closed dark / light off, 1 = closed with light on). */
void ui_oven_door_close_anim_to(uint8_t end_frame);

/* Animation frame source list, defined in generated/screens/gg_screen.c */
extern const void * screen_oven_animimg_src_list[];

/* Current oven door state (kOVEN_DoorClose / kOVEN_DoorOpen), defined in ui_Oven.c */
extern bool ui_oven_door_state;


#endif /* UI_OVEN_H_ */
