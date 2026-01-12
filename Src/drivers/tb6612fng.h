#ifndef TB6612FNG_H
#define TB6612FNG_H

// tb6612fng interface implementation using the pwm driver
#include <stdint.h>
typedef enum
{
    TB6612FNG_LEFT,
    TB6612FNG_RIGHT
} tb6612fng_e;

typedef enum
{
    TB6612FNG_MODE_REVERSE, // COUNTER-CLOCKWISE(CCW)
    TB6612FNG_MODE_FORMARD, // CLOCKWISE(CC)
    TB6612FNG_MODE_STOP,
} tb6612fng_mode_e;

void tb6612fng_init(void);
void tb6612fng_set_mode(tb6612fng_e tb6612fng, tb6612fng_mode_e mode);
void tb6612fng_set_pwm(tb6612fng_e tb6612fng, uint8_t duty_cycle);
#endif // TB6612FNG_H