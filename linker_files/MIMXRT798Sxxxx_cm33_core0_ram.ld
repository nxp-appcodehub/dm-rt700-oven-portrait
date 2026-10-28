/*
** ###################################################################
**     Processors:          MIMXRT798SGAWAR_cm33_core0
**                          MIMXRT798SGFOA_cm33_core0
**
**     Compiler:            GNU C Compiler
**     Reference manual:    iMXRT700RM Rev.2 DraftA, 05/2024
**     Version:             rev. 2.0, 2024-05-28
**
**     Abstract:
**         Linker file for the GNU C Compiler
**         XIP flash layout aligned with the MCUXpresso IDE generated
**         memory map for the RT700 Waveshare display application:
**           QSPI_FLASH     0x28000000  32M  (code XIP)
**           UI_RESOURCES   0x2a000000  32M  (LVGL images/fonts/const)
**           SRAM           0x20080000  512K (data/bss/stack/heap)
**           NCACHE_REGION  0x20100000  6M   (non-cacheable framebuffers)
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2026 NXP
**     SPDX-License-Identifier: BSD-3-Clause
**
** ###################################################################
*/

/* Entry Point */
ENTRY(Reset_Handler)

HEAP_SIZE    = DEFINED(__heap_size__)  ? __heap_size__  : 0x1000;
STACK_SIZE   = DEFINED(__stack_size__) ? __stack_size__ : 0x1800;

/* Specify the memory areas */
MEMORY
{
  m_flash               (RX)  : ORIGIN = 0x28000000, LENGTH = 0x02000000 /* 32M QSPI flash (alias QSPI_FLASH)  */
  m_ui_resources        (RX)  : ORIGIN = 0x2a000000, LENGTH = 0x02000000 /* 32M UI resources (alias Flash2)    */
  m_data                (RW)  : ORIGIN = 0x20080000, LENGTH = 0x00080000 /* 512K SRAM (alias RAM)              */
  m_ncache              (RW)  : ORIGIN = 0x20100000, LENGTH = 0x00600000 /* 6M non-cacheable (alias RAM2)      */
}

/* Define output sections */
SECTIONS
{
  .flash_config :
  {
    FILL(0xFF)
    __FLASH_BASE = .;
    KEEP(* (.flash_conf))     /* flash config section */
    . = 0x4000;
  } > m_flash

  /* The startup code and vector table go into flash (XIP) */
  .interrupts :
  {
    . = ALIGN(4);
    __VECTOR_TABLE = .;
    __Vectors = .;
    KEEP(*(.isr_vector))     /* Startup code */
    . = ALIGN(4);
  } > m_flash

  /* The program code and other data go into flash (XIP) */
  .text :
  {
    . = ALIGN(4);
    *(.text)                 /* .text sections (code) */
    *(.text*)                /* .text* sections (code) */
    *(.rodata)               /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)              /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
  } > m_flash

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > m_flash

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > m_flash

  .ctors :
  {
    __CTOR_LIST__ = .;
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    __CTOR_END__ = .;
  } > m_flash

  .dtors :
  {
    __DTOR_LIST__ = .;
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    __DTOR_END__ = .;
  } > m_flash

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } > m_flash

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } > m_flash

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > m_flash

  __etext = .;    /* define a global symbol at end of code */
  __DATA_ROM = .; /* Symbol is used by startup for data initialization */

  __VECTOR_RAM = __VECTOR_TABLE;
  __RAM_VECTOR_TABLE_SIZE_BYTES = 0x0;

  /* Large read-only UI resources (LVGL images/fonts) stay resident in the
     second flash bank and are read in place (not copied to RAM). */
  .ui_resources :
  {
    . = ALIGN(4);
    *(UIResources)
    *(DataQuickAccess)       /* LVGL LV_ATTRIBUTE_LARGE_CONST image maps */
    *(.rodata.$UI_RESOURCES)
    *(.rodata.$UI_RESOURCES.*)
    *(.text.$UI_RESOURCES)
    *(.text.$UI_RESOURCES.*)
    . = ALIGN(4);
  } > m_ui_resources

  /* Main DATA section (SRAM), loaded from flash */
  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __DATA_RAM = .;
    __data_start__ = .;      /* create a global symbol at data start */
    *(CodeQuickAccess)       /* CodeQuickAccess sections */
    *(.ramfunc*)
    *(RamFunction)
    *(vtable)
    *(.data)                 /* .data sections */
    *(.data*)                /* .data* sections */
    KEEP(*(.jcr*))
    . = ALIGN(4);
    __data_end__ = .;        /* define a global symbol at data end */
  } > m_data

  __NCACHE_REGION_START = ORIGIN(m_ncache);
  __NCACHE_REGION_SIZE  = LENGTH(m_ncache);
  __NDATA_ROM = __DATA_ROM + (__data_end__ - __data_start__); /* Symbol is used by startup for ncache data initialization */

  .ncache.init : AT(__NDATA_ROM)
  {
    __noncachedata_start__ = .;   /* create a global symbol at ncache data start */
    *(NonCacheable.init)
    . = ALIGN(4);
    __noncachedata_init_end__ = .;   /* create a global symbol at initialized ncache data end */
  } > m_ncache
  . = __noncachedata_init_end__;
  .ncache :
  {
    *(NonCacheable)
    . = ALIGN(4);
    __noncachedata_end__ = .;     /* define a global symbol at ncache data end */
  } > m_ncache

  __DATA_END = __DATA_ROM + (__data_end__ - __data_start__) + (__noncachedata_init_end__ - __noncachedata_start__);
  _image_size = __DATA_END - ORIGIN(m_flash);
  ASSERT(_image_size <= LENGTH(m_flash), "region m_flash overflowed with text and data")

  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    . = ALIGN(4);
    __START_BSS = .;
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    __END_BSS = .;
  } > m_data

  .stack :
  {
    . = ALIGN(8);
    . += STACK_SIZE;
  } > m_data

  .heap :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    __HeapBase = .;
    . += HEAP_SIZE;
    __HeapLimit = .;
    __heap_limit = .; /* Add for _sbrk */
  } > m_data

  /* Initializes stack on the end of block */
  __StackTop   = ORIGIN(m_data) + LENGTH(m_data);
  __StackLimit = __StackTop - STACK_SIZE;
  PROVIDE(__stack = __StackTop);

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
