#ifndef PWM_H
#define PWM_H

#include <stdint.h>

/* PWM carrier frequency for BLDC gate drive.
 * 20 kHz is above audio range, below switching loss threshold for typical MOSFETs. */
#define PWM_FREQ_HZ     20000U
#define PWM_PERIOD      ((uint16_t)(168000000UL / PWM_FREQ_HZ))  /* ARR value */

/* Dead-time to prevent high-side and low-side shoot-through.
 * 500 ns at 168 MHz = 84 clock cycles. (BDTR:DTG field, §17.4.18) */
#define PWM_DEADTIME_CYCLES  84U

void pwm_init(void);
void pwm_set_duty(uint8_t duty_percent);   /* 0–100 */
void pwm_enable(void);
void pwm_disable(void);

#endif /* PWM_H */
