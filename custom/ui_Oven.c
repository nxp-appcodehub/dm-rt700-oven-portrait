/*
 * Copyright 2022-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ui_Oven.h"

#include "gui_guider.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
uint16_t lv_obj_get_tile_id(lv_obj_t * obj);
static void oven_wheel_scroll_event_cb(lv_event_t * e);
static void ui_oven_anim_door(bool open, uint32_t delay);
static void oven_door_anim_exec_cb(void * var, int32_t idx);
/*******************************************************************************
 * Variables
 ******************************************************************************/
lv_obj_t * ui_Label_Target_Temp_Numer;

uint16_t Oven_Mode = kOVEN_ModeFFRE;
bool ui_oven_door_state = kOVEN_DoorClose;
bool ui_oven_light_state = kOVEN_lightOff;
bool ui_oven_fan_state = kOVEN_FanOn;

uint32_t prueba_pixel;

/*******************************************************************************
 * Function
 ******************************************************************************/
/*Complete Oven setting*/
void ui_oven_init (void)
{

	/*Oven target temperature*/
	ui_Label_Target_Temp_Numer = lv_label_create(guider_ui.screen.container_1_roller_image);

	lv_obj_set_width(ui_Label_Target_Temp_Numer, 101);
	lv_obj_set_height(ui_Label_Target_Temp_Numer, LV_SIZE_CONTENT);

	lv_obj_set_x(ui_Label_Target_Temp_Numer, -60);
	lv_obj_set_y(ui_Label_Target_Temp_Numer, 2);

	lv_obj_set_align(ui_Label_Target_Temp_Numer, LV_ALIGN_RIGHT_MID);

	lv_label_set_text(ui_Label_Target_Temp_Numer, "150");

	lv_obj_set_style_text_color(ui_Label_Target_Temp_Numer, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(ui_Label_Target_Temp_Numer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui_Label_Target_Temp_Numer, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui_Label_Target_Temp_Numer, &lv_font_Antonio_Regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_set_flex_flow(guider_ui.screen.container_1, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(guider_ui.screen.container_1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END);
	lv_obj_set_style_pad_row(guider_ui.screen.container_1, 22, 0);
	lv_obj_set_style_text_color(guider_ui.screen.container_1, lv_color_hex(0xff9110), 0);
	lv_obj_set_style_text_font(guider_ui.screen.container_1, &lv_font_Antonio_Regular_20, 0);
	lv_obj_add_flag(guider_ui.screen.container_1_roller_image, LV_OBJ_FLAG_IGNORE_LAYOUT | LV_OBJ_FLAG_FLOATING);

	lv_obj_add_event_cb(guider_ui.screen.container_1, oven_wheel_scroll_event_cb, LV_EVENT_SCROLL, ui_Label_Target_Temp_Numer);
	lv_obj_set_scroll_snap_y(guider_ui.screen.container_1, LV_SCROLL_SNAP_CENTER);

	uint32_t i;
	for(i = 0; i < 20; i++) {
		lv_obj_t * label = lv_label_create(guider_ui.screen.container_1);
		lv_label_set_text_fmt(label, "%d", 150 + i * 5);
		lv_obj_set_width(label, 80);
	}

	lv_obj_move_foreground(guider_ui.screen.container_1_roller_image);

	lv_obj_update_layout(guider_ui.screen.container_1);
	lv_obj_update_snap(guider_ui.screen.container_1, LV_ANIM_OFF);
	lv_obj_send_event(guider_ui.screen.container_1, LV_EVENT_SCROLL, NULL);

}

void ui_oven_set_Temperature(uint32_t temp)
{
	uint32_t i;

	for(i = 0; i < 20; i++)
	{
		if(temp == (150 + i*5))
		{
			break;
		}
	}

	lv_obj_scroll_to_y(guider_ui.screen.container_1, (-145 + 40 * i), LV_ANIM_ON);
}


/*
 * Number of frames in the oven door animation image list
 * (screen_oven_animimg_src_list defined in generated/screens/gg_screen.c).
 */
#define OVEN_DOOR_FRAME_COUNT   10
/* Duration in ms for a full open or close sweep. */
#define OVEN_DOOR_ANIM_TIME     1000

/*
 * Play the oven door image sequence forward (closed -> open).
 * The animation drives frame indices 0 .. OVEN_DOOR_FRAME_COUNT-1 and
 * updates the animimg image source on every step.
 */
void ui_oven_door_open_anim(void)
{
	lv_obj_t * img = guider_ui.screen.oven_animimg;
	lv_anim_t a;

	/* Cancel any running door animation on this object first. */
	lv_anim_del(img, oven_door_anim_exec_cb);

	lv_anim_init(&a);
	lv_anim_set_var(&a, img);
	lv_anim_set_exec_cb(&a, oven_door_anim_exec_cb);
	lv_anim_set_time(&a, OVEN_DOOR_ANIM_TIME);
	lv_anim_set_repeat_count(&a, 0);
	/* Forward: first frame to last frame. */
	lv_anim_set_values(&a, 0, OVEN_DOOR_FRAME_COUNT - 1);
	lv_anim_start(&a);

	ui_oven_door_state = kOVEN_DoorOpen;
}

/*
 * Play the oven door image sequence forward but start on a specific frame
 * (0 = closed dark, 1 = closed with light on). Used to avoid a dark-frame
 * flash when the interior light is already on before opening.
 */
void ui_oven_door_open_anim_from(uint8_t start_frame)
{
	lv_obj_t * img = guider_ui.screen.oven_animimg;
	lv_anim_t a;

	if(start_frame >= OVEN_DOOR_FRAME_COUNT) start_frame = OVEN_DOOR_FRAME_COUNT - 1;

	lv_anim_del(img, oven_door_anim_exec_cb);

	lv_anim_init(&a);
	lv_anim_set_var(&a, img);
	lv_anim_set_exec_cb(&a, oven_door_anim_exec_cb);
	lv_anim_set_time(&a, OVEN_DOOR_ANIM_TIME);
	lv_anim_set_repeat_count(&a, 0);
	/* Forward: from the requested start frame to the last (open) frame. */
	lv_anim_set_values(&a, start_frame, OVEN_DOOR_FRAME_COUNT - 1);
	lv_anim_start(&a);

	ui_oven_door_state = kOVEN_DoorOpen;
}

/*
 * Play the oven door image sequence backward (open -> closed).
 * The animation drives frame indices OVEN_DOOR_FRAME_COUNT-1 .. 0.
 */
void ui_oven_door_close_anim(void)
{
	lv_obj_t * img = guider_ui.screen.oven_animimg;
	lv_anim_t a;

	lv_anim_del(img, oven_door_anim_exec_cb);

	lv_anim_init(&a);
	lv_anim_set_var(&a, img);
	lv_anim_set_exec_cb(&a, oven_door_anim_exec_cb);
	lv_anim_set_time(&a, OVEN_DOOR_ANIM_TIME);
	lv_anim_set_repeat_count(&a, 0);
	/* Backward: last frame to the closed (dark) frame. */
	lv_anim_set_values(&a, OVEN_DOOR_FRAME_COUNT - 1, 0);
	lv_anim_start(&a);

	ui_oven_door_state = kOVEN_DoorClose;
}

/*
 * Play the oven door image sequence backward but stop on a specific end
 * frame (e.g. frame 1 = closed with light on, frame 0 = closed dark).
 * This lets the caller restore the light state without an image flash.
 */
void ui_oven_door_close_anim_to(uint8_t end_frame)
{
	lv_obj_t * img = guider_ui.screen.oven_animimg;
	lv_anim_t a;

	if(end_frame >= OVEN_DOOR_FRAME_COUNT) end_frame = OVEN_DOOR_FRAME_COUNT - 1;

	lv_anim_del(img, oven_door_anim_exec_cb);

	lv_anim_init(&a);
	lv_anim_set_var(&a, img);
	lv_anim_set_exec_cb(&a, oven_door_anim_exec_cb);
	lv_anim_set_time(&a, OVEN_DOOR_ANIM_TIME);
	lv_anim_set_repeat_count(&a, 0);
	/* Backward: last frame down to the requested end frame. */
	lv_anim_set_values(&a, OVEN_DOOR_FRAME_COUNT - 1, end_frame);
	lv_anim_start(&a);

	ui_oven_door_state = kOVEN_DoorClose;
}

/*******************************************************************************
 * Static function
 ******************************************************************************/

/*
 * Animation execution callback: maps the current animation value (frame index)
 * to the matching image in screen_oven_animimg_src_list and displays it.
 */
static void oven_door_anim_exec_cb(void * var, int32_t idx)
{
	lv_obj_t * img = (lv_obj_t *)var;

	if(idx < 0) idx = 0;
	if(idx >= OVEN_DOOR_FRAME_COUNT) idx = OVEN_DOOR_FRAME_COUNT - 1;

	lv_img_set_src(img, screen_oven_animimg_src_list[idx]);
}

static void oven_wheel_scroll_event_cb(lv_event_t * e)

{
	lv_obj_t * cont = lv_event_get_target(e);

	lv_area_t cont_a;
	lv_obj_get_coords(cont, &cont_a);
	lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

	int32_t inv = lv_obj_get_style_flex_track_place(cont, 0) == LV_FLEX_ALIGN_START ? 1 : -1;
	lv_coord_t y_min = LV_COORD_MAX;
	lv_obj_t * y_min_obj = NULL;
	lv_coord_t r = lv_obj_get_height(cont) * 55 / 100;
	uint32_t i;
	uint32_t child_cnt = lv_obj_get_child_cnt(cont);
	for(i = 0; i < child_cnt; i++) {
		lv_obj_t * child = lv_obj_get_child(cont, i);
		if(lv_obj_has_flag(child, LV_OBJ_FLAG_FLOATING)) continue;
		lv_area_t child_a;
		lv_obj_get_coords(child, &child_a);

		lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;
		lv_coord_t diff_y = child_y_center - cont_y_center;
		diff_y = LV_ABS(diff_y);
		if(diff_y < y_min) {
			y_min = diff_y;
			y_min_obj = child;
		}

		/*Get the x of diff_y on a circle.*/
		lv_coord_t x;
		/*If diff_y is out of the circle use the last point of the circle (the radius)*/
		if(diff_y >= r) {
			x = r;
		}
		else {
			/*Use Pythagoras theorem to get x from radius and y*/
			uint32_t x_sqr = r * r - diff_y * diff_y;
			lv_sqrt_res_t res;
			lv_sqrt(x_sqr, &res, 0x8000);   /*Use lvgl's built in sqrt root function*/
			x = r - res.i;
		}

		/*Translate the item by the calculated X coordinate*/
		lv_obj_set_style_translate_x(child, inv * x, 0);
	}

	lv_obj_t * large_label = lv_event_get_user_data(e);
	lv_label_set_text(large_label, lv_label_get_text(y_min_obj));
	prueba_pixel = lv_obj_get_scroll_y(guider_ui.screen.container_1);
}


