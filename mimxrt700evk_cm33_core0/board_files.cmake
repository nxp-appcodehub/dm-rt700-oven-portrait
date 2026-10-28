
# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause


mcux_add_source(
    SOURCES vglite_support/vglite_support.c
            vglite_support/vglite_support.h
)

mcux_add_include(
    INCLUDES vglite_support
)
mcux_add_configuration(
    CC "-DSDK_DEBUGCONSOLE=1"
    CX "-DSDK_DEBUGCONSOLE=1"
)


mcux_add_source(
    SOURCES mimxrt700evk/board.c
            mimxrt700evk/board.h
)

mcux_add_include(
    INCLUDES mimxrt700evk
)

mcux_add_source(
    SOURCES cm33_core0/clock_config.c
            cm33_core0/clock_config.h
)

mcux_add_include(
    INCLUDES cm33_core0
)
