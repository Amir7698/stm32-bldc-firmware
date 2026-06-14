/**
 * main.c — Entry point for STM32F407 BLDC motor controller
 *
 * Initialisation order matters:
 *   1. SystemInit (clocks) — called from startup.s before main(); here we
 *      just confirm the SysTick is running.
 *   2. gpio_init  — must precede uart_init/pwm_init because AF and clock
 *      enable writes happen here.
 *   3. uart_init  — needs APB1 clock ready (done in SystemInit).
 *   4. pwm_init   — enables TIM1, starts counter, registers TIM1 IRQ.
 *   5. motor_init — purely software, no hardware dependency.
 *
 * Main loop runs at ~1 kHz (limited by SysTick resolution). The motor_run()
 * function handles all state transitions and UART reporting.
 */

#include "../include/system_clock.h"
#include "../include/gpio.h"
#include "../include/uart.h"
#include "../include/pwm.h"
#include "../include/motor.h"

int main(void) {

    gpio_init();    /* GPIOs, EXTI, SYSCFG */
    uart_init();    /* USART2 debug */
    pwm_init();     /* TIM1 three-phase PWM */
    motor_init();   /* State machine init */

    /* Blink LED once to signal successful boot */
    gpio_led_set(1);
    delay_ms(200);
    gpio_led_set(0);

    /* ── Main loop ─────────────────────────────────────────────────────── */
    while (1) {
        motor_run();   /* State machine tick + UART report */

        /* Toggle LED at 1 Hz to confirm main loop is alive.
         * LED period = 500 ms per half-cycle × 2 = 1 Hz. */
        static uint32_t led_tick = 0U;
        if ((get_tick_ms() - led_tick) >= 500U) {
            gpio_led_toggle();
            led_tick = get_tick_ms();
        }
    }
}
