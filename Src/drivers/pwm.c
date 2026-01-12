#include "pwm.h"
#include "assert_handler.h"
#include "defines.h"
#include "stm32l4xx.h"
#include "io.h"
#include <stdbool.h>
#include <assert.h>

#define TIMER_FREQ_ms (CLOCK_FREQ_80MHZ / TIMER_PRESCALER)
#define BASE_PERIOD_FREQ_HZ (20000U)
#define BASE_PWM_PERIOD (TIMER_FREQ_ms / BASE_PERIOD_FREQ_HZ)
#define TIMER_PSC ((80U / 2U) - 1U)
#define TIMER_ARR (BASE_PWM_PERIOD - 1)
static_assert(BASE_PWM_PERIOD == 100, "PWM period should be 100");

static bool initialized = false;
static const struct io_config pwm_config = { .mode = IO_MODE_ALTFN,
                                             .pupd = IO_NO_PUPD,
                                             .speed = IO_SPEED_VERY_HIGH,
                                             .type = IO_TYPE_PP,
                                             .af = IO_AF_2 };

struct pwm_channel_config
{
    bool enable;
    volatile uint32_t *ccr;
    volatile uint32_t *ccer;
};
static struct pwm_channel_config pwm_tb6612fng[] = {
    [PWM_TB6612FNG_LEFT_CH] = { .enable = false, .ccr = &TIM3->CCR1, .ccer = &TIM3->CCER },
    [PWM_TB6612FNG_RIGHT_CH] = { .enable = false, .ccr = &TIM3->CCR2, .ccer = &TIM3->CCER },
};
void pwm_init(void)
{
    ASSERT(!initialized);
    struct io_config current_config;
    io_get_io_config(IO_MOTOR_PWM_LEFT, &current_config);
    ASSERT(io_compare_io_config(&current_config, &pwm_config));
    io_get_io_config(IO_MOTOR_PWM_RIGHT, &current_config);
    ASSERT(io_compare_io_config(&current_config, &pwm_config));

    RCC->APB1ENR1 |= 0x1 << 1;

    TIM3->PSC = TIMER_PSC;
    TIM3->ARR = TIMER_ARR;
    // PC6 -> TIM3_CH1 PC7 -> TIM3_CH2
    // set capture mode as output
    TIM3->CCMR1 = (TIM3->CCMR1 & ~0x3) | (TIM3->CCMR1 & ~(0x3 << 8));
    // set mode as PWM 1 for channel 1 and 2
    TIM3->CCMR1 = (TIM3->CCMR1 & ~(0xF << 4)) | (TIM3->CCMR1 & ~(0xF << 12));
    TIM3->CCMR1 = (TIM3->CCMR1 | (0x6 << 4)) | (TIM3->CCMR1 | (0x6 << 12));
    // enable preload register
    TIM3->CCMR1 = (TIM3->CCMR1 | (0x1 << 3)) | (TIM3->CCMR1 | (0x1 << 11));
    // clear capture compare resgiter 1
    TIM3->CCR1 = 0;
    TIM3->CCR2 = 0;
    // enable capture compare one after configuring it for channels 1 and 2
    // TIM3->CCER |= 0x1 | (0x1 << 4);
    // enable auto reload preload register
    TIM3->CR1 |= 0x1 << 7;
    // enable timer update generation
    TIM3->EGR |= 0x1;

    initialized = true;
}

static bool timer_all_pwm_chs_disabled(void)
{
    bool disabled = true;
    uint8_t i;
    for (i = 0; i < ARRAY_SIZE(pwm_tb6612fng); i++) {
        if (pwm_tb6612fng[i].enable) {
            disabled = false;
        }
    }
    return disabled;
}
static bool pwm_enabled = false;
static void pwm_enable(bool enable)
{
    if (pwm_enabled != enable) {
        pwm_enabled = enable;
        TIM3->CNT = 0;
        if (enable) {
            TIM3->CR1 |= 0x1;
        } else {
            TIM3->CR1 &= ~0x1;
        }
    }
}
// batterys supply about 8V fully charged
// motors are rated at 6V
// need to scale down pwm to allow for max voltage to be 6V
// value should never be zero
static inline uint8_t scale_pwm_down(uint8_t duty_cycle_percent)
{
    return duty_cycle_percent != 1
        ? (3 * duty_cycle_percent) / 4
        : duty_cycle_percent; // do fraction muliplication to avoid floating point errors
}
static void pwm_channel_enable(bool enable, pwm_ch_e channel)
{
    // check if they are not the same so if they are no need to reconfigure registers
    if (pwm_tb6612fng[channel].enable != enable) {
        pwm_tb6612fng[channel].enable = enable;
        if (enable) {
            *pwm_tb6612fng[channel].ccer |= (channel == PWM_TB6612FNG_LEFT_CH) ? 0x1 : (0x1 << 4);
            pwm_enable(true);
        } else {
            *pwm_tb6612fng[channel].ccer &= (channel == PWM_TB6612FNG_LEFT_CH) ? ~0x1 : ~(0x1 << 4);
            if (timer_all_pwm_chs_disabled()) {
                pwm_enable(false);
            }
        }
    }
}
// IO_MOTOR_PWM_LEFT = IO_PC_6,
// IO_MOTOR_PWM_RIGHT = IO_PC_7,
void pwm_set_duty_cycle(pwm_ch_e channel, uint8_t duty_cycle_percent)
{
    ASSERT(initialized);
    ASSERT(duty_cycle_percent <= 100);
    const bool enable = duty_cycle_percent > 0;
    if (enable) {
        *pwm_tb6612fng[channel].ccr = scale_pwm_down(duty_cycle_percent);
    }
    pwm_channel_enable(enable, channel);
}