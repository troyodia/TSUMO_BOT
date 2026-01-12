#include "ir_remote.h"
#include "assert_handler.h"
#include "io.h"
#include "ring_buffer.h"
#include "defines.h"
#include "stm32l4xx.h"
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

#define TIMER_FREQ_ms (CLOCK_FREQ_80MHZ_ms / TIMER_PRESCALER) // 2Mhz
// we want the period to be 1ms
#define TIMER_INTERRUPT_PERIOD_ms (1u)
#define TIMER_INTERRUPT_TICKS (TIMER_FREQ_ms * TIMER_INTERRUPT_PERIOD_ms)
#define TIMER_TIMEOUT_ms (150U)
// actual timer prescaler
// formula Required frequency = clock freq /(PSC + 1)
// derive PSC formula from above, 80Mhz clock freq, 2Mhz required freq
#define TIMER_PSC ((80U / 2U) - 1U)
#define IR_BUFFER_SIZE (10u)

// just for referent Timer Period (seconds) = (ARR + 1) * (PSC + 1) / Timer_Clock_Frequency

static_assert(TIMER_INTERRUPT_TICKS <= 0xFFFFU, "Ticks cannot be larger than 16bits");
static bool initialized = false;
static uint8_t timer_ms = 0;
static uint16_t pulse_count = 0;

static uint8_t buffer[IR_BUFFER_SIZE];
static struct ring_buffer ir_buffer = {
    .size = IR_BUFFER_SIZE, .head = 0, .tail = 0, .buffer = buffer
};

static union {
    struct
    {
        // cppcheck-suppress unusedStructMember
        uint8_t inv_command;
        uint8_t command;
        // cppcheck-suppress unusedStructMember
        uint8_t inv_address;
        // cppcheck-suppress unusedStructMember
        uint8_t address;
    } ir_decoded;
    uint32_t ir_message_raw;
} ir_message;

// static union ir_message ir_message;
static void start_timer(void)
{
    // reset the current count value
    TIM2->CNT = 0;
    // set as up-counter and enable the counter
    TIM2->CR1 = (TIM2->CR1 & ~(0x1 << 4)) | 0x1;
    timer_ms = 0;
}
static void stop_timer(void)
{
    // disable the counter
    TIM2->CR1 &= ~0x1;
}

static void timer_init(void)
{
    // set the TIM2 clock enabled
    RCC->APB1ENR1 |= 0x1;
    // set the required prescaler to get 80Mhz down to 2Mhz
    TIM2->PSC = TIMER_PSC;
    // set the timer increment cycle to be 1ms
    TIM2->ARR = TIMER_INTERRUPT_TICKS - 1;

    // enable interrupt UIE
    TIM2->DIER |= 0x1;

    //  enable the counter
    // start_timer();
    TIM2->SR &= ~0x1;

    NVIC_EnableIRQ(TIM2_IRQn);
}
static inline bool is_ir_bit_pulse(uint16_t pulse)
{
    return (pulse >= 3) && (pulse <= 34);
}
static inline bool is_ir_pulses_complete(uint16_t pulse)
{
    return (pulse == 34) || ((pulse > 36) && IS_PULSE_ODD(pulse));
}

static void isr_pulse_PA_8(void)
{
    stop_timer();
    pulse_count++;
    if (is_ir_bit_pulse(pulse_count)) {
        ir_message.ir_message_raw <<= 1;
        ir_message.ir_message_raw |= (timer_ms < 2) ? 0 : 1;
    }
    if (is_ir_pulses_complete(pulse_count)) {
        ring_buffer_put(&ir_buffer, ir_message.ir_decoded.command);
    }
    start_timer();
}
void ir_remote_init(void)
{
    ASSERT(!initialized);
    io_interrupt_config((io_e)IO_PA_8, isr_pulse_PA_8, IO_FALLING_TRIGGER, IO_EXTI_N_0);
    io_enable_interrupt(IO_EXTI_9_5_LINE);
    timer_init();
    initialized = true;
}
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & 0x1) {
        // set a timeout for the timer to conserve energy
        if (timer_ms < TIMER_TIMEOUT_ms) {
            timer_ms++;
        } else {
            stop_timer();
            timer_ms = 0;
            pulse_count = 0;
            ir_message.ir_message_raw = 0;
        }
        // clear the update intterupt status
        TIM2->SR &= ~0x1;
    }
}
ir_cmd_e ir_remote_get_cmd(void)
{
    // Disable ir pin interrupt to account for clash with a call of this function
    // this is to ensure the right pulse is being got from the buffer
    io_disable_interrupt(IO_EXTI_9_5_LINE);
    ir_cmd_e cmd = IR_CMD_NONE;
    if (!ring_buffer_empty(&ir_buffer)) {
        cmd = ring_buffer_get(&ir_buffer);
    }
    io_enable_interrupt(IO_EXTI_9_5_LINE);

    return cmd;
}
const char *ir_get_cmd_str(ir_cmd_e cmd)
{
    switch (cmd) {
    case IR_CMD_0:
        return "0";
        break;
    case IR_CMD_1:
        return "1";
        break;
    case IR_CMD_2:
        return "2";
        break;
    case IR_CMD_3:
        return "3";
        break;
    case IR_CMD_4:
        return "4";
        break;
    case IR_CMD_5:
        return "5";
        break;
    case IR_CMD_6:
        return "6";
        break;
    case IR_CMD_7:
        return "7";
        break;
    case IR_CMD_8:
        return "8";
        break;
    case IR_CMD_9:
        return "9";
        break;
    case IR_CMD_ASTERISK:
        return "*";
        break;
    case IR_CMD_POUND:
        return "#";
        break;
    case IR_CMD_UP:
        return "UP";
        break;
    case IR_CMD_DOWN:
        return "DOWN";
        break;
    case IR_CMD_LEFT:
        return "LEFT";
        break;
    case IR_CMD_RIGHT:
        return "RIGHT";
        break;
    case IR_CMD_OK:
        return "OK";
        break;
    default:
        return "NONE";
        break;
    }
}
