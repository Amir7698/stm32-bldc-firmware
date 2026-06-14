/**
 * startup_stm32f407.s — Minimal Cortex-M4 startup for STM32F407
 *
 * Responsibilities:
 *  1. Place vector table at address 0x08000000 (start of Flash)
 *  2. Copy .data section from Flash LMA to SRAM VMA
 *  3. Zero-fill .bss section
 *  4. Call SystemInit then main()
 *
 * ARM Cortex-M4 boot sequence (ARM DDI0439 §2.3.4):
 *  - On reset, core reads SP from address 0x00000000, then PC from 0x00000004.
 *  - Flash is aliased at 0x00000000 on STM32F4 by default (BOOT pins = 00).
 */

    .syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

/* ── Stack and heap sizes ───────────────────────────────────────────────── */
    .equ    Stack_Size, 0x400   /* 1 KiB stack — adequate for this firmware */
    .equ    Heap_Size,  0x200   /* 512 B heap */

    .section .stack, "w", %nobits
    .align  3
    .globl  __StackTop
    .globl  __StackLimit
__StackLimit:
    .space  Stack_Size
__StackTop:

    .section .heap, "w", %nobits
    .align  3
    .globl  __HeapBase
    .globl  __HeapLimit
__HeapBase:
    .space  Heap_Size
__HeapLimit:

/* ── Vector table — must be at 0x08000000 ────────────────────────────── */
    .section .isr_vector, "a", %progbits
    .type   g_pfnVectors, %object

g_pfnVectors:
    .word   __StackTop              /* Initial stack pointer (SP) */
    .word   Reset_Handler           /* Reset handler */
    .word   NMI_Handler
    .word   HardFault_Handler
    .word   MemManage_Handler
    .word   BusFault_Handler
    .word   UsageFault_Handler
    .word   0                       /* Reserved */
    .word   0
    .word   0
    .word   0
    .word   SVC_Handler
    .word   DebugMon_Handler
    .word   0
    .word   PendSV_Handler
    .word   SysTick_Handler
    /* External interrupts (IRQ0–IRQ81 for STM32F407) */
    .word   WWDG_IRQHandler
    .word   PVD_IRQHandler
    .word   TAMP_STAMP_IRQHandler
    .word   RTC_WKUP_IRQHandler
    .word   FLASH_IRQHandler
    .word   RCC_IRQHandler
    .word   EXTI0_IRQHandler        /* IRQ6  — direction signal */
    .word   EXTI1_IRQHandler        /* IRQ7  — enable signal */
    .word   EXTI2_IRQHandler
    .word   EXTI3_IRQHandler
    .word   EXTI4_IRQHandler
    .word   DMA1_Stream0_IRQHandler
    .word   DMA1_Stream1_IRQHandler
    .word   DMA1_Stream2_IRQHandler
    .word   DMA1_Stream3_IRQHandler
    .word   DMA1_Stream4_IRQHandler
    .word   DMA1_Stream5_IRQHandler
    .word   DMA1_Stream6_IRQHandler
    .word   ADC_IRQHandler
    .word   CAN1_TX_IRQHandler
    .word   CAN1_RX0_IRQHandler
    .word   CAN1_RX1_IRQHandler
    .word   CAN1_SCE_IRQHandler
    .word   EXTI9_5_IRQHandler
    .word   TIM1_BRK_TIM9_IRQHandler
    .word   TIM1_UP_TIM10_IRQHandler  /* IRQ25 — PWM period update */
    .word   TIM1_TRG_COM_TIM11_IRQHandler
    .word   TIM1_CC_IRQHandler
    .word   TIM2_IRQHandler
    .word   TIM3_IRQHandler
    .word   TIM4_IRQHandler
    .word   I2C1_EV_IRQHandler
    .word   I2C1_ER_IRQHandler
    .word   I2C2_EV_IRQHandler
    .word   I2C2_ER_IRQHandler
    .word   SPI1_IRQHandler
    .word   SPI2_IRQHandler
    .word   USART1_IRQHandler
    .word   USART2_IRQHandler       /* IRQ38 — debug UART */
    .word   Default_Handler         /* Remaining slots — alias to Default */

/* ── Default handler — infinite loop (aids debugger breakpoints) ─────── */
    .section .text.Default_Handler, "ax", %progbits
    .weak   Default_Handler
    .type   Default_Handler, %function
Default_Handler:
    b       Default_Handler         /* Spin — inspect LR in debugger to find caller */

/* ── Weak aliases so unimplemented ISRs link without error ───────────── */
    .macro  WEAK_IRQ name
    .weak   \name
    .thumb_set \name, Default_Handler
    .endm

    WEAK_IRQ NMI_Handler
    WEAK_IRQ HardFault_Handler
    WEAK_IRQ MemManage_Handler
    WEAK_IRQ BusFault_Handler
    WEAK_IRQ UsageFault_Handler
    WEAK_IRQ SVC_Handler
    WEAK_IRQ DebugMon_Handler
    WEAK_IRQ PendSV_Handler
    WEAK_IRQ SysTick_Handler
    WEAK_IRQ WWDG_IRQHandler
    WEAK_IRQ PVD_IRQHandler
    WEAK_IRQ TAMP_STAMP_IRQHandler
    WEAK_IRQ RTC_WKUP_IRQHandler
    WEAK_IRQ FLASH_IRQHandler
    WEAK_IRQ RCC_IRQHandler
    WEAK_IRQ EXTI0_IRQHandler
    WEAK_IRQ EXTI1_IRQHandler
    WEAK_IRQ EXTI2_IRQHandler
    WEAK_IRQ EXTI3_IRQHandler
    WEAK_IRQ EXTI4_IRQHandler
    WEAK_IRQ DMA1_Stream0_IRQHandler
    WEAK_IRQ DMA1_Stream1_IRQHandler
    WEAK_IRQ DMA1_Stream2_IRQHandler
    WEAK_IRQ DMA1_Stream3_IRQHandler
    WEAK_IRQ DMA1_Stream4_IRQHandler
    WEAK_IRQ DMA1_Stream5_IRQHandler
    WEAK_IRQ DMA1_Stream6_IRQHandler
    WEAK_IRQ ADC_IRQHandler
    WEAK_IRQ CAN1_TX_IRQHandler
    WEAK_IRQ CAN1_RX0_IRQHandler
    WEAK_IRQ CAN1_RX1_IRQHandler
    WEAK_IRQ CAN1_SCE_IRQHandler
    WEAK_IRQ EXTI9_5_IRQHandler
    WEAK_IRQ TIM1_BRK_TIM9_IRQHandler
    WEAK_IRQ TIM1_UP_TIM10_IRQHandler
    WEAK_IRQ TIM1_TRG_COM_TIM11_IRQHandler
    WEAK_IRQ TIM1_CC_IRQHandler
    WEAK_IRQ TIM2_IRQHandler
    WEAK_IRQ TIM3_IRQHandler
    WEAK_IRQ TIM4_IRQHandler
    WEAK_IRQ I2C1_EV_IRQHandler
    WEAK_IRQ I2C1_ER_IRQHandler
    WEAK_IRQ I2C2_EV_IRQHandler
    WEAK_IRQ I2C2_ER_IRQHandler
    WEAK_IRQ SPI1_IRQHandler
    WEAK_IRQ SPI2_IRQHandler
    WEAK_IRQ USART1_IRQHandler
    WEAK_IRQ USART2_IRQHandler

/* ── Reset handler ───────────────────────────────────────────────────── */
    .section .text.Reset_Handler, "ax", %progbits
    .weak   Reset_Handler
    .type   Reset_Handler, %function

Reset_Handler:
    /* Step 1: Initialise stack pointer — already loaded from vector table by HW */

    /* Step 2: Copy .data section (initialised globals) from Flash to SRAM.
     * Linker exports __data_start__, __data_end__ (VMA in SRAM) and
     * __etext (LMA in Flash immediately after .text). */
    ldr     r0, =__data_start__
    ldr     r1, =__data_end__
    ldr     r2, =__etext
    movs    r3, #0
copy_data:
    cmp     r0, r1
    bge     zero_bss
    ldr     r4, [r2, r3]
    str     r4, [r0, r3]
    adds    r3, r3, #4
    b       copy_data

    /* Step 3: Zero-fill .bss (uninitialised globals). */
zero_bss:
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    movs    r2, #0
zero_bss_loop:
    cmp     r0, r1
    bge     call_main
    str     r2, [r0]
    adds    r0, r0, #4
    b       zero_bss_loop

    /* Step 4: Call SystemInit (clocks) then main(). */
call_main:
    bl      SystemInit
    bl      main
    /* main() should never return; if it does, spin */
hang:
    b       hang

    .size   Reset_Handler, .-Reset_Handler
