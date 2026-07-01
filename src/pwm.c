/**
 * pwm.c — TIM1 three-phase complementary PWM for BLDC gate drive
 *
 * TIM1 is the only "advanced-control" timer on STM32F4, which means it
 * natively supports:
 *   • 3 complementary channel pairs (CHx + CHxN)
 *   • Dead-time insertion between CHx and CHxN (prevents bridge shoot-through)
 *   • Break input (fault shutdown)
 *   • Repetition counter (halve update-event rate for synchronous sampling)
 *
 * Center-aligned mode (CMS = 01) is chosen over edge-aligned because it:
 *   • Halves the effective harmonic content at the motor windings
 *   • Naturally centres the PWM on-time, which simplifies current sensing
 *   • Makes all three channels switch simultaneously at the top of the triangle
 *
 * PWM period with center-align: f_pwm = TIM_CLK / (2 × ARR)
 * → ARR = TIM_CLK / (2 × f_pwm) = 168e6 / (2 × 20000) = 4200
 */

#include "../cmsis/stm32f4xx.h"
#include "../include/pwm.h"
#include "../include/system_clock.h"

/* Computed ARR for 20 kHz center-aligned PWM at 168 MHz */
#define ARR_VAL     ((uint16_t)(TIM1_CLK_HZ / (2UL * PWM_FREQ_HZ)))   /* 4200 */

void pwm_init(void) {

    /* ── Enable TIM1 clock ─────────────────────────────────────────────── */
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    (void)RCC->APB2ENR;   /* Dummy read — clears the AHB bus pipeline before register access */

    /* ── CR1: center-aligned mode, no auto-preload yet ───────────────────
     * CMS = 0b01: center-aligned, interrupt on counting down (not needed here).
     * ARPE = 1: shadow register for ARR — new period takes effect at next update,
     * prevents glitches when duty changes at mid-cycle. */
    TIM1->CR1 = TIM_CR1_CMS_CENTER1   /* Center-aligned mode 1 */
              | TIM_CR1_ARPE;          /* ARR preload enable */

    /* ── Prescaler: no division, TIM1 runs at full 168 MHz ─────────────── */
    TIM1->PSC = 0U;

    /* ── Auto-reload: sets the PWM period ───────────────────────────────── */
    TIM1->ARR = ARR_VAL;   /* 4200 → 20 kHz center-aligned */

    /* ── Repetition counter: fire update event every 2 PWM periods ───────
     * RCR = 1 means update event fires after 2 counter periods.
     * Useful for triggering ADC at consistent current measurement point.
     * For this firmware we set 0 (every period) for simplicity. */
    TIM1->RCR = 0U;

    /* ── CCMR1: CH1 and CH2 output compare mode ─────────────────────────
     * PWM mode 1: output high when CNT < CCR, low otherwise.
     * OC1PE/OC2PE: preload CCR so duty changes don't cause mid-cycle glitches. */
    TIM1->CCMR1 = TIM_CCMR_OC1M_PWM1   /* CH1: PWM mode 1 */
                | TIM_CCMR_OC1PE        /* CH1 CCR preload */
                | TIM_CCMR_OC2M_PWM1   /* CH2: PWM mode 1 */
                | TIM_CCMR_OC2PE;       /* CH2 CCR preload */

    /* ── CCMR2: CH3 output compare mode ─────────────────────────────────
     * Only CH3 is used here (CH4 is not connected to motor phases). */
    TIM1->CCMR2 = TIM_CCMR_OC3M_PWM1   /* CH3: PWM mode 1 */
                | TIM_CCMR_OC3PE;       /* CH3 CCR preload */

    /* ── Initial duty cycle: 0% (CCR = 0) ───────────────────────────────── */
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCR3 = 0U;

    /* ── CCER: enable all six outputs (CHx and CHxN) ─────────────────────
     * Both normal (CHx) and complementary (CHxN) outputs active.
     * Polarity bits left 0 = active-high, matching typical N-MOSFET gate drive. */
    TIM1->CCER = TIM_CCER_CC1E  | TIM_CCER_CC1NE
               | TIM_CCER_CC2E  | TIM_CCER_CC2NE
               | TIM_CCER_CC3E  | TIM_CCER_CC3NE;

    /* ── BDTR: dead-time and break configuration ─────────────────────────
     * Dead-time (DTG[7:0]): protects against simultaneous conduction.
     *   DTG[7:6] = 0b00 → dead-time = DTG[5:0] × Tdts  (Tdts = 1/168 MHz ≈ 5.95 ns)
     *   DTG = 84 → 84 × 5.95 ns ≈ 500 ns — appropriate for most MOSFETs.
     * OSSR = 1: force CHxN low when timer is stopped (safe state for gate drive).
     * OSSI = 1: force CHxN low when MOE=0 and timer is running.
     * MOE must be set to enable outputs — if cleared by break, set only after fault clear. */
    TIM1->BDTR = (PWM_DEADTIME_CYCLES & 0xFF)   /* DTG = 84 cycles */
               | TIM_BDTR_OSSR                  /* Off-state run */
               | TIM_BDTR_OSSI;                 /* Off-state idle */
    /* MOE is NOT set here — pwm_enable() sets it to allow controlled start */

    /* ── EGR: force register preload shadow copy ─────────────────────────
     * UG generates a software update event, transferring ARR/CCR/PSC preload
     * values to their shadow registers so the first PWM period is correct. */
    TIM1->EGR = TIM_EGR_UG;

    /* Enable TIM1 update interrupt for RPM bookkeeping */
    TIM1->DIER = TIM_DIER_UIE;
    NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1);
    NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

    /* ── CR1 CEN: start the counter ─────────────────────────────────────── */
    TIM1->CR1 |= TIM_CR1_CEN;
}

void pwm_set_duty(uint8_t duty_percent) {
    if (duty_percent > 100U) duty_percent = 100U;

    /* Scale duty to CCR: CCR = (duty / 100) × ARR */
    uint16_t ccr = (uint16_t)(((uint32_t)duty_percent * ARR_VAL) / 100UL);

    /* All three phases carry the same duty — 6-step commutation would vary
     * which channels are active, but that logic lives in the motor layer. */
    TIM1->CCR1 = ccr;
    TIM1->CCR2 = ccr;
    TIM1->CCR3 = ccr;
}

void pwm_enable(void) {
    /* Set Main Output Enable — gate drive outputs become active */
    TIM1->BDTR |= TIM_BDTR_MOE;
}

void pwm_disable(void) {
    /* Clear MOE — all outputs go to idle state as defined by OSSI/OSSR */
    TIM1->BDTR &= ~TIM_BDTR_MOE;
}
