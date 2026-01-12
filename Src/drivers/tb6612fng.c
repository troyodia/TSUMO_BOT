#include "tb6612fng.h"
#include "pwm.h"
#include "defines.h"
#include "assert_handler.h"
#include "io.h"
#include <stdbool.h>
#include <assert.h>

struct t6612fng_chs
{
    io_e ch1;
    io_e ch2;
};
static struct t6612fng_chs t6612fng_ch_pins[] = {
    [TB6612FNG_LEFT] = { .ch1 = IO_MOTOR_LEFT_CH1, .ch2 = IO_MOTOR_LEFT_CH2 },
    [TB6612FNG_RIGHT] = { .ch1 = IO_MOTOR_RIGHT_CH1, .ch2 = IO_MOTOR_RIGHT_CH2 }
};
const struct io_config tb6612fng_ch_config = { .mode = IO_MODE_OUPUT,
                                               .pupd = IO_NO_PUPD,
                                               .speed = IO_SPEED_VERY_HIGH,
                                               .type = IO_TYPE_PP,
                                               .af = IO_AF_NONE };
static bool initialized = false;

static void tb6612fng_assert_io_config(void)
{
    struct io_config current_config;
    io_get_io_config(IO_MOTOR_LEFT_CH1, &current_config);
    ASSERT(io_compare_io_config(&current_config, &tb6612fng_ch_config));
    io_get_io_config(IO_MOTOR_LEFT_CH2, &current_config);
    ASSERT(io_compare_io_config(&current_config, &tb6612fng_ch_config));
    io_get_io_config(IO_MOTOR_RIGHT_CH1, &current_config);
    ASSERT(io_compare_io_config(&current_config, &tb6612fng_ch_config));
    io_get_io_config(IO_MOTOR_RIGHT_CH2, &current_config);
    ASSERT(io_compare_io_config(&current_config, &tb6612fng_ch_config));
}

void tb6612fng_init(void)
{
    ASSERT(!initialized);
    tb6612fng_assert_io_config();
    pwm_init();
    initialized = true;
}
void tb6612fng_set_mode(tb6612fng_e tb6612fng, tb6612fng_mode_e mode)
{
    switch (mode) {
    case TB6612FNG_MODE_FORMARD:
        io_set_output(t6612fng_ch_pins[tb6612fng].ch1, IO_OUT_HIGH);
        io_set_output(t6612fng_ch_pins[tb6612fng].ch2, IO_OUT_LOW);
        break;
    case TB6612FNG_MODE_REVERSE:
        io_set_output(t6612fng_ch_pins[tb6612fng].ch1, IO_OUT_LOW);
        io_set_output(t6612fng_ch_pins[tb6612fng].ch2, IO_OUT_HIGH);
        break;
    case TB6612FNG_MODE_STOP:
        io_set_output(t6612fng_ch_pins[tb6612fng].ch1, IO_OUT_LOW);
        io_set_output(t6612fng_ch_pins[tb6612fng].ch2, IO_OUT_LOW);
        break;
    }
}
static_assert(TB6612FNG_LEFT == (int)PWM_TB6612FNG_LEFT_CH,
              "The enums that holds each tb6612fng motor driver must match for both drivers");
static_assert(TB6612FNG_RIGHT == (int)PWM_TB6612FNG_RIGHT_CH,
              "The enums that holds each tb6612fng motor driver must match for both drivers");

void tb6612fng_set_pwm(tb6612fng_e tb6612fng, uint8_t duty_cycle)
{
    pwm_set_duty_cycle((pwm_ch_e)tb6612fng, duty_cycle);
}