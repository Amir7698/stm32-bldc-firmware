/**
 * gpio.c — GPIO and EXTI configuration for BLDC controller
 *
 * All TIM1 PWM outputs use Alternate Function 1 (AF1) on STM32F407
 * (see datasheet Table 9 — Alternate function mapping).
 * USART2 uses AF7 on PA2/PA3.
 */

#include "../cmsis/stm32f4xx.h"
#include "../include/gpio.h"

void gpio_init(void) {

    /* ── Enable GPIO clocks ────────────────────────────────────────────
     * Peripheral clocks must be enabled before any register access;
     * reading/writing registers of a clock-gated peripheral returns 0
     * and writes are silently discarded. */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN   /* GPIOA: PWM CH1/2/3, USART2, DIR, EN */
                  | RCC_AHB1ENR_GPIOBEN   /* GPIOB: PWM CH1N/2N/3N */
                  | RCC_AHB1ENR_GPIOCEN;  /* GPIOC: status LED */
    (void)RCC->AHB1ENR;                   /* Dummy read — ensure clock is active before use */

    /* ── PA8, PA9, PA10: TIM1_CH1/2/3 (high-side PWM) ──────────────────
     * MODER = 0b10 (AF), OSPEEDR = 0b11 (very high, needed for 20 kHz edges),
     * AFR[1] pins 8–10 = AF1 (TIM1). */
    GPIOA->MODER   |= (GPIO_MODER_AF << (8*2))    /* PA8  → AF */
                    | (GPIO_MODER_AF << (9*2))    /* PA9  → AF */
                    | (GPIO_MODER_AF << (10*2));  /* PA10 → AF */

    GPIOA->OSPEEDR |= (GPIO_OSPEEDR_VHIGH << (8*2))   /* PA8  very high speed */
                    | (GPIO_OSPEEDR_VHIGH << (9*2))   /* PA9  very high speed */
                    | (GPIO_OSPEEDR_VHIGH << (10*2)); /* PA10 very high speed */

    /* AFR[1] covers pins 8–15. Each pin uses 4 bits.
     * AF1 = 0x1 for TIM1 (datasheet Table 9). */
    GPIOA->AFR[1] |= (1UL << ((8-8)*4))   /* PA8  = AF1 */
                   | (1UL << ((9-8)*4))   /* PA9  = AF1 */
                   | (1UL << ((10-8)*4)); /* PA10 = AF1 */

    /* ── PB13, PB14, PB15: TIM1_CH1N/2N/3N (low-side PWM) ─────────────
     * Same AF1 but on GPIOB. Datasheet Table 9 confirms TIM1 CH1N–3N on PB13–15. */
    GPIOB->MODER   |= (GPIO_MODER_AF << (13*2))
                    | (GPIO_MODER_AF << (14*2))
                    | (GPIO_MODER_AF << (15*2));

    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_VHIGH << (13*2))
                    | (GPIO_OSPEEDR_VHIGH << (14*2))
                    | (GPIO_OSPEEDR_VHIGH << (15*2));

    GPIOB->AFR[1] |= (1UL << ((13-8)*4))   /* PB13 = AF1 */
                   | (1UL << ((14-8)*4))   /* PB14 = AF1 */
                   | (1UL << ((15-8)*4));  /* PB15 = AF1 */

    /* ── PA2 (USART2_TX), PA3 (USART2_RX): AF7 ─────────────────────────
     * Datasheet Table 9: USART2 is AF7 on PA2/PA3. */
    GPIOA->MODER   |= (GPIO_MODER_AF << (2*2))    /* PA2 = AF (TX) */
                    | (GPIO_MODER_AF << (3*2));   /* PA3 = AF (RX) */

    GPIOA->OSPEEDR |= (GPIO_OSPEEDR_HIGH << (2*2))
                    | (GPIO_OSPEEDR_HIGH << (3*2));

    GPIOA->AFR[0] |= (7UL << (2*4))   /* PA2 = AF7 (USART2_TX) */
                   | (7UL << (3*4));  /* PA3 = AF7 (USART2_RX) */

    /* ── PA0 (Direction), PA1 (Enable): digital inputs with pull-ups ────
     * External signals — pull-up ensures defined level when line floats. */
    GPIOA->MODER  &= ~((3UL << (0*2)) | (3UL << (1*2)));  /* PA0, PA1 = input */
    GPIOA->PUPDR  |= (1UL << (0*2))   /* PA0 pull-up */
                   | (1UL << (1*2));  /* PA1 pull-up */

    /* ── PC13: Status LED (Nucleo LD2, active low) ─────────────────────── */
    GPIOC->MODER  |= (GPIO_MODER_OUTPUT << (13*2));
    GPIOC->OTYPER &= ~(1UL << 13);                /* Push-pull */
    GPIOC->BSRR    = (1UL << (13 + 16));          /* Start LED off (set high = off) */

    /* ── SYSCFG + EXTI: route PA0/PA1 to EXTI line 0/1 ─────────────────
     * SYSCFG must be clocked before EXTICR can be written. */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    (void)RCC->APB2ENR;

    /* EXTICR[0] selects source for EXTI0 and EXTI1.
     * 0b0000 = PA, 0b0001 = PB, etc. (RM0090 §9.3.3). */
    SYSCFG->EXTICR[0] &= ~(0xFFUL);  /* EXTI0 = PA0, EXTI1 = PA1 */

    /* Both rising and falling edges — we track the actual pin state in ISR */
    EXTI->RTSR |= (1UL << 0) | (1UL << 1);   /* Rising trigger on EXTI0, EXTI1 */
    EXTI->FTSR |= (1UL << 0) | (1UL << 1);   /* Falling trigger */
    EXTI->IMR  |= (1UL << 0) | (1UL << 1);   /* Unmask interrupts */

    /* Enable EXTI0 and EXTI1 in NVIC at priority 2 (lower than SysTick=0) */
    NVIC_SetPriority(EXTI0_IRQn, 2);
    NVIC_SetPriority(EXTI1_IRQn, 2);
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

void gpio_led_set(uint8_t on) {
    if (on) {
        GPIOC->BSRR = (1UL << 13);          /* Set PC13 → LED on (active-high variant) */
    } else {
        GPIOC->BSRR = (1UL << (13 + 16));   /* Reset PC13 → LED off */
    }
}

void gpio_led_toggle(void) {
    GPIOC->ODR ^= (1UL << 13);
}
