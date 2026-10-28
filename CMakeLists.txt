
# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22.0)

cmake_path(APPEND conf_file_path ${CMAKE_CURRENT_LIST_DIR} ${board}_${core_id} prj.conf)
set(CONF_FILE ${conf_file_path} ${CONF_FILE})
include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(mimxrt700evk_lvgl_guider_cm33_core0 LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/lvgl_examples/lvgl_guider)

include(${SdkRootDirPath}/CMakeLists.txt)


include("${board}_${core_id}/board_files.cmake")

# ---------------------------------------------------------------------------
# Flash configuration selection (compile-time #define)
#
# The wrapper flash_config/flash_config.c pulls in the correct variant based
# on the USE_FRDM_FLASH_CONFIG macro defined at the top of that file:
#   - USE_FRDM_FLASH_CONFIG 0 : NXP MIMXRT700-EVK (flash_config/evk)
#   - USE_FRDM_FLASH_CONFIG 1 : NXP FRDM-RT700    (flash_config/frdm)
#
# To switch boards, edit the #define in flash_config/flash_config.c.
# ---------------------------------------------------------------------------

# The MIMXRT700-EVK board port already provides its own flash_config.c which
# defines the 'flash_config' symbol. Remove it so our variant wrapper below is
# the single definition (otherwise the linker reports a multiple definition).
mcux_project_remove_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES examples/_boards/mimxrt700evk/flash_config/flash_config.c
)

mcux_add_source(
    SOURCES flash_config/flash_config.c
)

# ---------------------------------------------------------------------------
# Display / LVGL / touch support override
#
# The SDK board port and lvgl_sdk provide default display_support.c and
# lvgl_support.c. Remove them and use the project-local versions (customized
# for the Waveshare 7" A panel) together with the panel driver sources.
# ---------------------------------------------------------------------------
mcux_project_remove_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES examples/_boards/mimxrt700evk/display_support.c
            examples/lvgl_examples/lvgl_sdk/lvgl_support/rt700/lvgl_support.c
)

mcux_add_source(
    SOURCES mimxrt700evk_cm33_core0/mimxrt700evk/display_support.c
            mimxrt700evk_cm33_core0/mimxrt700evk/display_support.h
            mimxrt700evk_cm33_core0/mimxrt700evk/lvgl_support.c
            mimxrt700evk_cm33_core0/mimxrt700evk/lvgl_support.h
            panel/waveshare7a/display/fsl_waveshare7touchA.c
            panel/waveshare7a/display/fsl_waveshare7touchA.h
            panel/waveshare7a/touch/fsl_gt9271.c
            panel/waveshare7a/touch/fsl_gt9271.h
)

mcux_add_include(
    INCLUDES mimxrt700evk_cm33_core0/mimxrt700evk
             panel/waveshare7a/display
             panel/waveshare7a/touch
)

# The SDK board port dir (examples/_boards/mimxrt700evk) also ships a
# display_support.h that uses the same include guard (_DISPLAY_SUPPORT_H_) as
# our project copy. Because that SDK dir is earlier on the include path, its
# header wins and our WAVESHARE_7TOUCHA panel definition never takes effect.
# Remove the SDK board dir and re-add it so it is searched AFTER our project
# include dirs, letting our display_support.h win while keeping board.h etc.
mcux_project_remove_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES examples/_boards/mimxrt700evk
)
mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES examples/_boards/mimxrt700evk
)





mcux_add_include(
    INCLUDES custom
             generated
             generated/assets
             generated/assets/fonts
             generated/assets/images
             generated/events
             generated/screens
             lvgl_examples
)

mcux_add_source(
    SOURCES lvgl_examples/FreeRTOSConfig_frag.h
            lvgl_examples/lvgl_demo_utils.c
            lvgl_examples/lvgl_demo_utils.h
            lvgl_examples/lvgl_freertos.c
            lvgl_examples/lvgl_freertos.h
            lvgl_guider.c
            custom/lv_conf_ext.h
)

mcux_add_source(
    SOURCES mimxrt700evk_lvgl_examples/app.h
            mimxrt700evk_lvgl_examples/hardware_init.c
            mimxrt700evk_lvgl_examples/pin_mux.c
            mimxrt700evk_lvgl_examples/pin_mux.h
)

mcux_add_include(
    INCLUDES lvgl_guider
             mimxrt700evk_lvgl_examples
)
mcux_add_iar_configuration(
    CC "--diag_suppress Pa082,Pa050,Pa092"
    CX "--diag_suppress Pa082,Pa050,Pa092"
)

mcux_add_iar_configuration(
    TARGETS debug release flash_debug flash_release
    LD "--redirect __iar_sh_stdout=__iar_sh_stdout_swo"
)

mcux_add_macro(
    CC "-DGCID_REV_CID=gc555/0x423_ECO \
        -DCUSTOM_VGLITE_MEMORY_CONFIG=0"
)

mcux_add_configuration(
        CC "-DSDK_I2C_BASED_COMPONENT_USED=1\
            -DBOARD_ENABLE_PSRAM_CACHE=0\
            -DSSD1963_DATA_WITDH=8\
            -DFLEXIO_MCULCD_DATA_BUS_WIDTH=8\
            -DMCUX_DBI_LEGACY=0\
            -DDBI_USE_MIPI_PANEL=1\
            -DSDK_DEBUGCONSOLE_UART=1"
)

mcux_add_armgcc_configuration(
        CC "-Wno-error=unused-variable"
        CX "-Wno-error=unused-variable"
    )
#Avoid errors of unused functions
mcux_add_armgcc_configuration(
        CC "-Wno-error=unused-function"
        CX "-Wno-error=unused-function"
    )
#Avoid errors of format type mismatch (e.g. uint32_t with %d)
mcux_add_armgcc_configuration(
        CC "-Wno-error=format="
        CX "-Wno-error=format="
    )

mcux_remove_iar_linker_script(
    BASE_PATH ${SdkRootDirPath}
    TARGETS debug release
    LINKER ${device_root}/RT/RT700/MIMXRT798S/iar/MIMXRT798Sxxxx_cm33_core0_ram.icf
)

mcux_remove_mdk_linker_script(
    BASE_PATH ${SdkRootDirPath}
    TARGETS debug release
    LINKER ${device_root}/RT/RT700/MIMXRT798S/arm/MIMXRT798Sxxxx_cm33_core0_ram.scf
)

mcux_remove_armgcc_linker_script(
    BASE_PATH ${SdkRootDirPath}
    TARGETS debug release
    LINKER ${device_root}/RT/RT700/MIMXRT798S/gcc/MIMXRT798Sxxxx_cm33_core0_ram.ld
)


mcux_add_iar_linker_script(
    LINKER linker_files/MIMXRT798Sxxxx_cm33_core0_ram.icf
    TARGETS debug release
)

mcux_add_mdk_linker_script(
    LINKER linker_files/MIMXRT798Sxxxx_cm33_core0_ram.scf
    TARGETS debug release
)

mcux_add_armgcc_linker_script(
    LINKER linker_files/MIMXRT798Sxxxx_cm33_core0_ram.ld
    TARGETS debug release
)
mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)


mcux_add_include(INCLUDES .)

# BEGIN AUTO-ADDED BY MCUXPRESSO EXTENSION
mcux_add_source(BASE_PATH ${CMAKE_CURRENT_LIST_DIR}
	SOURCES "custom/custom.c"
	"custom/lv_font_Antonio_Regular_20.c"
	"custom/lv_font_Antonio_Regular_22.c"
	"custom/lv_font_Antonio_Regular_30.c"
	"custom/lv_font_Antonio_Regular_50.c"
	"custom/ui_Oven.c")
# END AUTO-ADDED BY MCUXPRESSO EXTENSION

# BEGIN AUTO-ADDED BY MCUXPRESSO EXTENSION
mcux_add_source(BASE_PATH ${CMAKE_CURRENT_LIST_DIR}
	SOURCES "generated/gg_utils.c")
# END AUTO-ADDED BY MCUXPRESSO EXTENSION

# BEGIN AUTO-ADDED BY MCUXPRESSO EXTENSION
mcux_add_source(BASE_PATH ${CMAKE_CURRENT_LIST_DIR}
	SOURCES "generated/assets/fonts/lv_font_Antonio_22.c"
	"generated/assets/fonts/lv_font_Antonio_119.c"
	"generated/assets/fonts/lv_font_montserratMedium_22.c")
# END AUTO-ADDED BY MCUXPRESSO EXTENSION

# BEGIN AUTO-ADDED BY MCUXPRESSO EXTENSION
mcux_add_source(BASE_PATH ${CMAKE_CURRENT_LIST_DIR}
	SOURCES "generated/assets/images/arrow_down_28_14_36x37_RGB565A8.c"
	"generated/assets/images/arrow_left_18_35_22x96_RGB565A8.c"
	"generated/assets/images/arrow_right_18_35_22x96_RGB565A8.c"
	"generated/assets/images/arrow_up_28_14_36x37_RGB565A8.c"
	"generated/assets/images/BTN_oven_134_51_81x51_RGB565A8.c"
	"generated/assets/images/BTN_oven_orange_134_51_81x51_RGB565A8.c"
	"generated/assets/images/icn_oven_fan_rear_orange_62_62_51x51_RGB565A8.c"
	"generated/assets/images/icn_oven_fan_rear_white_62_62_51x51_RGB565A8.c"
	"generated/assets/images/oven_close_dark_450x470_RGB565A8.c"
	"generated/assets/images/oven_open_light_450x470_RGB565A8.c"
	"generated/assets/images/text_oven_15_62_18x103_RGB565A8.c"
	"generated/assets/images/title_bg_h_654_130_103x595_RGB565A8.c"
	"generated/assets/images/wheel_bg_317x613_RGB565A8.c")
# END AUTO-ADDED BY MCUXPRESSO EXTENSION

# BEGIN AUTO-ADDED BY MCUXPRESSO EXTENSION
mcux_add_source(BASE_PATH ${CMAKE_CURRENT_LIST_DIR}
	SOURCES "generated/events/gg_event_layer_bottom.c"
	"generated/events/gg_event_layer_sys.c"
	"generated/events/gg_event_layer_top.c"
	"generated/events/gg_event_screen.c")
# END AUTO-ADDED BY MCUXPRESSO EXTENSION

# BEGIN AUTO-ADDED BY MCUXPRESSO EXTENSION
mcux_add_source(BASE_PATH ${CMAKE_CURRENT_LIST_DIR}
	SOURCES "generated/screens/gg_layer_bottom.c"
	"generated/screens/gg_layer_sys.c"
	"generated/screens/gg_layer_top.c"
	"generated/screens/gg_screen.c")
# END AUTO-ADDED BY MCUXPRESSO EXTENSION

# BEGIN AUTO-ADDED BY MCUXPRESSO EXTENSION
mcux_add_source(BASE_PATH ${CMAKE_CURRENT_LIST_DIR}
	SOURCES "generated/assets/images/oven_menu_door_1_450x470_RGB565A8.c"
	"generated/assets/images/oven_menu_door_2_450x470_RGB565A8.c"
	"generated/assets/images/oven_menu_door_3_450x470_RGB565A8.c"
	"generated/assets/images/oven_menu_door_4_450x470_RGB565A8.c"
	"generated/assets/images/oven_menu_door_5_450x470_RGB565A8.c"
	"generated/assets/images/oven_menu_door_6_450x470_RGB565A8.c"
	"generated/assets/images/oven_menu_door_7_450x470_RGB565A8.c"
	"generated/assets/images/oven_menu_door_8_450x470_RGB565A8.c")
# END AUTO-ADDED BY MCUXPRESSO EXTENSION
