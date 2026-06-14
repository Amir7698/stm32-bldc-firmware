#ifndef SYSTEM_CLOCK_H
#define SYSTEM_CLOCK_H

#include <stdint.h>

/* System clock after PLL configuration */
#define SYSCLK_HZ       168000000UL   /* 168 MHz — STM32F407 maximum */
#define APB1_HZ          42000000UL   /* APB1 divider /4 */
#define APB2_HZ          84000000UL   /* APB2 divider /2 */

/* TIM1 is on APB2. When APB2 divider != 1, timer clock = 2 × APB2 */
#define TIM1_CLK_HZ     (APB2_HZ * 2UL)  /* 168 MHz */

extern volatile uint32_t g_systick_ms;

void SystemInit(void);
void delay_ms(uint32_t ms);
uint32_t get_tick_ms(void);

#endif /* SYSTEM_CLOCK_H */
