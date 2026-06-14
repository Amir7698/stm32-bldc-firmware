/**
 * stm32f407.ld — Linker script for STM32F407VGT6
 *
 * Memory layout (RM0090 §2.3 Table 1):
 *   Flash: 1 MB  at 0x08000000  (code + read-only data)
 *   SRAM1: 112 KB at 0x20000000  (data + BSS + stack)
 *   SRAM2: 16 KB  at 0x2001C000  (not used here)
 *
 * Section placement rationale:
 *   .isr_vector must be first in Flash — the Cortex-M4 boot ROM reads
 *   the initial SP from [0x08000000] and PC from [0x08000004].
 */

ENTRY(Reset_Handler)

MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 112K
}

SECTIONS {

    /* ── Interrupt vector table — must reside at start of Flash ── */
    .isr_vector : {
        . = ALIGN(4);
        KEEP(*(.isr_vector))   /* KEEP prevents --gc-sections from discarding it */
        . = ALIGN(4);
    } >FLASH

    /* ── Code and read-only data ─────────────────────────────── */
    .text : {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.glue_7)             /* ARM/Thumb interworking stubs */
        *(.glue_7t)
        . = ALIGN(4);
        __etext = .;           /* LMA end of .text — used by startup to find .data image */
    } >FLASH

    /* ── ARM exception tables (required for C++ unwinding; harmless in C) */
    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
    .ARM       : {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    /* ── Initialised data — lives in Flash, copied to SRAM by startup ── */
    .data : {
        . = ALIGN(4);
        __data_start__ = .;    /* Startup code copies from __etext to here */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
    } >SRAM AT>FLASH           /* VMA in SRAM, LMA in Flash */

    /* ── Uninitialised data — zero-filled by startup ─────────── */
    .bss : {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } >SRAM

    /* ── Stack at top of SRAM (grows downward) ───────────────── */
    ._stack : {
        . = ALIGN(8);          /* 8-byte aligned per AAPCS */
        . = . + 0x400;         /* 1 KiB stack */
        . = ALIGN(8);
        __StackTop = .;
    } >SRAM

    /* Discard debug info sections to keep image small */
    /DISCARD/ : {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
