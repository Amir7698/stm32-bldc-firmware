#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

typedef enum {
    MOTOR_IDLE    = 0,
    MOTOR_RUNNING = 1,
    MOTOR_FAULT   = 2,
} MotorState;

typedef struct {
    MotorState  state;
    uint8_t     duty_percent;    /* 0–100 */
    uint16_t    rpm;             /* estimated from commutation period */
    uint8_t     direction;       /* 0 = forward, 1 = reverse */
    uint8_t     enabled;         /* 1 = enable pin asserted */
    uint32_t    fault_timestamp; /* ms tick when fault was latched */
} MotorContext;

extern MotorContext g_motor;

void motor_init(void);
void motor_run(void);              /* call from main loop */
void motor_set_enable(uint8_t en);
void motor_set_direction(uint8_t dir);
void motor_fault_latch(void);

#endif /* MOTOR_H */
