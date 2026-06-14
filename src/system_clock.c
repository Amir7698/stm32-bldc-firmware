/**
 * system_clock.c — Configure STM32F407 PLL to 168 MHz
 *
 * Clock tree target (RM0090 §6.3):
 *   HSE (8 MHz oscillator on Nucleo) → PLL → SYSCLK = 168 MHz
 *   AHB  divider = /1  → HCLK  = 168 MHz
 *   APB1 divider = /4  → PCLK1 =  42 MHz  (max 42 MHz per datasheet)
 *   APB2 divider = /2  → PCLK2 =  84 MHz  (max 84 MHz per datasheet)
 *
 * PLL formula (RM0090 §6.3.2):
 *   VCO_in  = HSE / PLLM  = 8 / 8 = 1 MHz    (target: 1–2 MHz)
 *   VCO_out = VCO_in * PLLN = 1 * 336 = 336 MHz  (target: 192–432 MHz)
 *   SYSCLK  = VCO_out / PLLP = 336 / 2 = 168 MHz
 *   USB_clk = VCO_out / PLLQ = 336 / 7 = 48 MHz  (USB requires exactly 48 MHz)
 */

#include "../cmsis/stm32f4xx.h"
#include "../include/system_clock.h"

volatile uint32_t g_systick_ms = 0;

void SystemInit(void) {

    /* ── 1. Enable HSE and wait for it to stabilise ─────────────────────
     * HSE is needed as a stable reference for the PLL. Using HSI (internal)
     * would give ±1% accuracy — too imprecise for UART baud rates. */
    RCC->CR |= (1UL << 16);                /* RCC_CR_HSEON bit 16 */
    while (!(RCC->CR & (1UL << 17)));      /* Wait for HSERDY (bit 17) */

    /* ── 2. Set Flash wait states BEFORE increasing clock speed ─────────
     * At 168 MHz, Vcc = 3.3 V, Flash requires 5 wait states (Table 10).
     * Prefetch, instruction-cache, and data-cache all reduce effective latency. */
    FLASH_ACR = FLASH_ACR_LATENCY_5WS
              | FLASH_ACR_PRFTEN           /* Prefetch buffer enable */
              | FLASH_ACR_ICEN             /* Instruction cache enable */
              | FLASH_ACR_DCEN;            /* Data cache enable */
    while ((FLASH_ACR & 0x7UL) != FLASH_ACR_LATENCY_5WS); /* Confirm write */

    /* ── 3. Configure PLL ────────────────────────────────────────────────
     * PLLCFGR layout (RM0090 §6.3.2):
     *   [5:0]  PLLM  = 8   (divide HSE by 8 → 1 MHz VCO input)
     *   [14:6] PLLN  = 336 (multiply VCO input → 336 MHz VCO output)
     *   [17:16] PLLP = 0b00 = /2 → 168 MHz SYSCLK
     *   [22]   PLLSRC = 1  (HSE source)
     *   [27:24] PLLQ = 7   (USB 48 MHz: 336/7) */
    RCC->PLLCFGR = (8UL  << 0)    /* PLLM  = 8 */
                 | (336UL << 6)   /* PLLN  = 336 */
                 | (0UL  << 16)   /* PLLP  = /2 (encoded as 0b00) */
                 | (1UL  << 22)   /* PLLSRC: HSE */
                 | (7UL  << 24);  /* PLLQ  = 7 */

    /* ── 4. Enable PLL and wait for lock ───────────────────────────────── */
    RCC->CR |= (1UL << 24);                /* RCC_CR_PLLON bit 24 */
    while (!(RCC->CR & (1UL << 25)));      /* Wait for PLLRDY (bit 25) */

    /* ── 5. Set bus dividers BEFORE switching to PLL ────────────────────
     * APB1 must not exceed 42 MHz, APB2 must not exceed 84 MHz.
     * CFGR[7:4] HPRE  = 0b0000 → AHB  /1
     * CFGR[12:10] PPRE1 = 0b101 → APB1 /4
     * CFGR[15:13] PPRE2 = 0b100 → APB2 /2 */
    RCC->CFGR |= (0b0000UL << 4)   /* HPRE:  AHB /1  */
              |  (0b101UL  << 10)  /* PPRE1: APB1 /4 */
              |  (0b100UL  << 13); /* PPRE2: APB2 /2 */

    /* ── 6. Switch SYSCLK source to PLL ─────────────────────────────────
     * CFGR[1:0] SW = 0b10 selects PLL. Poll SWS[3:2] to confirm switch. */
    RCC->CFGR = (RCC->CFGR & ~0x3UL) | 0x2UL;  /* SW = PLL */
    while ((RCC->CFGR & (0x3UL << 2)) != (0x2UL << 2)); /* Wait for SWS = PLL */

    /* ── 7. Configure SysTick for 1 ms interrupts ───────────────────────
     * SysTick counts down from LOAD to 0, then fires an interrupt.
     * At 168 MHz: LOAD = 168000 - 1 gives exactly 1 ms period. */
    SysTick->LOAD = (SYSCLK_HZ / 1000UL) - 1UL; /* 167999 */
    SysTick->VAL  = 0UL;                          /* Clear current value */
    SysTick->CTRL = SYSTICK_CTRL_CLKSOURCE        /* Use processor clock */
                  | SYSTICK_CTRL_TICKINT           /* Enable tick interrupt */
                  | SYSTICK_CTRL_ENABLE;           /* Start counting */
}

void SysTick_Handler(void) {
    g_systick_ms++;
}

uint32_t get_tick_ms(void) {
    return g_systick_ms;
}

void delay_ms(uint32_t ms) {
    uint32_t start = g_systick_ms;
    while ((g_systick_ms - start) < ms);  /* Wraps safely with unsigned arithmetic */
}
