/**
 * motor.c — BLDC state machine and interrupt handlers
 *
 * State transitions:
 *
 *   ┌──────┐  enable asserted   ┌─────────┐
 *   │ IDLE │ ──────────────────► │ RUNNING │
 *   └──────┘                    └─────────┘
 *      ▲                             │
 *      │  fault cleared              │ enable deasserted OR fault
 *      │                             ▼
 *      └──────────────────────── ┌───────┐
 *                                │ FAULT │
 *                                └───────┘
 *
 * FAULT is latched — it requires a deliberate enable-toggle to clear,
 * preventing automatic restart after an overcurrent event.
 *
 * RPM estimation:
 *   TIM1 fires an update interrupt every PWM period (50 µs at 20 kHz).
 *   A back-EMF zero-crossing counter would give commutation period, but
 *   without a comparator circuit we approximate RPM from a fixed duty-to-RPM
 *   curve (open-loop). Production firmware would use Hall sensors here.
 */

#include "../cmsis/stm32f4xx.h"
#include "../include/motor.h"
#include "../include/pwm.h"
#include "../include/uart.h"
#include "../include/system_clock.h"

/* Fault auto-reset lockout: must stay in FAULT for at least 2 s */
#define FAULT_LOCKOUT_MS    2000U

/* UART report interval */
#define REPORT_INTERVAL_MS  200U

MotorContext g_motor = {
    .state       = MOTOR_IDLE,
    .duty_percent = 0U,
    .rpm         = 0U,
    .direction   = 0U,
    .enabled     = 0U,
    .fault_timestamp = 0U,
};

static uint32_t s_last_report_ms = 0U;

/* ── Internal helpers ──────────────────────────────────────────────────── */

static void enter_idle(void) {
    pwm_set_duty(0U);
    pwm_disable();
    g_motor.state        = MOTOR_IDLE;
    g_motor.duty_percent = 0U;
    g_motor.rpm          = 0U;
}

static void enter_running(void) {
    g_motor.state        = MOTOR_RUNNING;
    g_motor.duty_percent = 50U;   /* Startup ramp at 50% */
    pwm_set_duty(g_motor.duty_percent);
    pwm_enable();
}

static void enter_fault(void) {
    pwm_disable();                /* Immediately kill gate drive */
    pwm_set_duty(0U);
    g_motor.state            = MOTOR_FAULT;
    g_motor.duty_percent     = 0U;
    g_motor.rpm              = 0U;
    g_motor.fault_timestamp  = get_tick_ms();
}

/* ── Public API ──────────────────────────────────────────────────────── */

void motor_init(void) {
    /* Nothing to do — state is statically initialised above */
}

void motor_fault_latch(void) {
    if (g_motor.state != MOTOR_FAULT) {
        enter_fault();
    }
}

void motor_set_enable(uint8_t en) {
    g_motor.enabled = en ? 1U : 0U;
}

void motor_set_direction(uint8_t dir) {
    g_motor.direction = dir ? 1U : 0U;
    /* Direction change while running: return to IDLE to avoid back-EMF spike */
    if (g_motor.state == MOTOR_RUNNING) {
        enter_idle();
    }
}

/* Open-loop RPM estimate: linear approximation duty → RPM.
 * For a real motor this would be replaced with Hall sensor period measurement. */
static uint16_t estimate_rpm(uint8_t duty) {
    /* Assume max RPM = 3000 at 100% duty, linear below */
    return (uint16_t)(((uint32_t)duty * 3000UL) / 100UL);
}

void motor_run(void) {
    switch (g_motor.state) {

        case MOTOR_IDLE:
            if (g_motor.enabled) {
                enter_running();
            }
            break;

        case MOTOR_RUNNING:
            if (!g_motor.enabled) {
                enter_idle();
                break;
            }
            g_motor.rpm = estimate_rpm(g_motor.duty_percent);
            break;

        case MOTOR_FAULT:
            /* Stay in FAULT until lockout expires AND enable has been deasserted */
            if (!g_motor.enabled &&
                (get_tick_ms() - g_motor.fault_timestamp) > FAULT_LOCKOUT_MS) {
                enter_idle();
            }
            break;
    }

    /* Periodic UART report regardless of state */
    uint32_t now = get_tick_ms();
    if ((now - s_last_report_ms) >= REPORT_INTERVAL_MS) {
        uart_send_frame(g_motor.rpm, g_motor.duty_percent, (uint8_t)g_motor.state);
        s_last_report_ms = now;
    }
}

/* ── Interrupt Service Routines ─────────────────────────────────────── */

/* EXTI0_IRQHandler — PA0 direction input changed */
void EXTI0_IRQHandler(void) {
    if (EXTI->PR & (1UL << 0)) {           /* Confirm EXTI0 caused this IRQ */
        EXTI->PR = (1UL << 0);             /* Clear pending bit by writing 1 (RM0090 §10.3.6) */

        uint8_t dir = (GPIOA->IDR >> 0) & 1U;  /* Read actual pin level */
        motor_set_direction(dir);
    }
}

/* EXTI1_IRQHandler — PA1 enable input changed */
void EXTI1_IRQHandler(void) {
    if (EXTI->PR & (1UL << 1)) {
        EXTI->PR = (1UL << 1);             /* Clear pending */

        uint8_t en = (GPIOA->IDR >> 1) & 1U;   /* Read actual pin level */
        motor_set_enable(en);
    }
}

/* TIM1_UP_TIM10_IRQHandler — fires every PWM period (50 µs)
 * Used as a heartbeat; production firmware would measure commutation here. */
void TIM1_UP_TIM10_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR &= ~TIM_SR_UIF;   /* Clear update interrupt flag */
        /* Hook: Hall sensor edge counting would go here */
    }
}
