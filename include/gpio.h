#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/*
 * Pin assignment (Nucleo-F407 compatible):
 *   PA8  — TIM1_CH1  (Phase U high-side)
 *   PB13 — TIM1_CH1N (Phase U low-side)
 *   PA9  — TIM1_CH2  (Phase V high-side)
 *   PB14 — TIM1_CH2N (Phase V low-side)
 *   PA10 — TIM1_CH3  (Phase W high-side)
 *   PB15 — TIM1_CH3N (Phase W low-side)
 *   PA2  — USART2_TX  (debug)
 *   PA3  — USART2_RX  (debug)
 *   PA0  — Direction input (EXTI0, rising+falling)
 *   PA1  — Enable input   (EXTI1, rising+falling)
 *   PC13 — Status LED (Nucleo on-board LED)
 */

void gpio_init(void);
void gpio_led_set(uint8_t on);
void gpio_led_toggle(void);

#endif /* GPIO_H */
