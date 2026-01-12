#ifndef PWM_H
#define PWM_H
#include <stdint.h>
// PWM driver for motor application
typedef enum
{
    PWM_TB6612FNG_LEFT_CH,
    PWM_TB6612FNG_RIGHT_CH // PWM signal for channel A and B of right motor driver (right motors)
} pwm_ch_e;

void pwm_init(void);
void pwm_set_duty_cycle(pwm_ch_e channel, uint8_t duty_cycle_percent);
#endif // PWM_H