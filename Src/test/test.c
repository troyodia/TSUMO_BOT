#include "../drivers/io.h"
#include "../drivers/mcu_init.h"
#include "../common/assert_handler.h"
#include "../common/trace.h"
#include "../drivers/led.h"
#include "../common/defines.h"
#include "../drivers/uart.h"
#include "../drivers/ir_remote.h"
#include "../drivers/pwm.h"
#include "../drivers/tb6612fng.h"
#include "../drivers/adc.h"
#include "../drivers/qre1113.h"
#include "../drivers/i2c.h"
#include "../drivers/urm09.h"
#include "../app/drive.h"
#include "../app/line.h"
#include "../app/enemy.h"
static const io_e io_pins[] = { IO_I2C_SDA,           IO_I2C_SCL,
                                IO_LD_FRONT_LEFT,     IO_LD_BACK_LEFT,
                                IO_UART_TX,           IO_UART_RX,
                                IO_LD_FRONT_RIGHT,    IO_XSHUT_LEFT,
                                IO_XSHUT_FRONT_LEFT,  IO_XSHUT_RIGHT,
                                IO_XSHUT_FRONT_RIGHT, IO_XSHUT_FRONT,
                                IO_LD_BACK_RIGHT,     IO_RANGE_SENSOR_INT_FRONT,
                                IO_MOTOR_LEFT_CH1,    IO_MOTOR_LEFT_CH2,
                                IO_MOTOR_RIGHT_CH1,   IO_MOTOR_RIGHT_CH2,
                                IO_MOTOR_PWM_LEFT,    IO_MOTOR_PWM_RIGHT,
                                IO_IR_REMOTE,         IO_TEST_LED };
static void test_setup(void)
{
    mcu_init();
}
SUPPRESS_UNUSED
static void test_assert(void)
{
    test_setup();
    ASSERT(0);
}

SUPPRESS_UNUSED
static void test_blink_led(void)
{
    test_setup();
    led_init();
    volatile int j;
    led_state_e led_state = LED_STATE_OFF;
    while (1) {
        led_state = (led_state == LED_STATE_OFF) ? LED_STATE_ON : LED_STATE_OFF;
        led_set(LED_TEST, led_state);
        BUSY_WAIT_ms(80)
    }
}
SUPPRESS_UNUSED
static void test_uart_interrupt(void)
{
    test_setup();
    uart_init();
    volatile int j;
    while (1) {
        // printf("Hello boy %d\n", 20);
        _putchar('h');
        _putchar('h');
        _putchar('h');
        _putchar('h');
        _putchar('\n');
        BUSY_WAIT_ms(60);
    }
}
SUPPRESS_UNUSED
static void test_trace(void)
{
    test_setup();
    trace_init();
    volatile int j;
    while (1) {
        TRACE("TEST TRACE %d", 1);
        BUSY_WAIT_ms(60);
    }
}
SUPPRESS_UNUSED
static void test_nucleo_board_io_pins_output(void)
{
    test_setup();

    const struct io_config output_config = { .mode = IO_MODE_OUPUT,
                                             .pupd = IO_NO_PUPD,
                                             .speed = IO_SPEED_VERY_HIGH,
                                             .type = IO_TYPE_PP,
                                             .af = IO_AF_NONE };
    volatile unsigned int i;
    volatile unsigned int j;
    for (i = 0; i < sizeof(io_pins) / sizeof(io_pins[0]); i++) {
        io_configure(io_pins[i], &output_config);
    }
    while (1) {
        for (i = 0; i < sizeof(io_pins) / sizeof(io_pins[0]); i++) {
            io_set_output(io_pins[i], IO_OUT_HIGH);
            BUSY_WAIT_ms(80) io_set_output(io_pins[i], IO_OUT_LOW);
        }
    }
}
SUPPRESS_UNUSED
static void test_nucleo_board_io_pins_input(void)
{
    test_setup();
    led_init();

    const struct io_config input_config = { .mode = IO_MODE_INPUT,
                                            .pupd = IO_PORT_PU,
                                            .speed = IO_SPEED_LOW,
                                            .type = IO_TYPE_PP,
                                            .af = IO_AF_NONE };
    volatile unsigned int i;
    volatile unsigned int j;

    for (i = 0; i < sizeof(io_pins) / sizeof(io_pins[0]); i++) {
        if (io_pins[i] == IO_TEST_LED) { // pin for nucleo board onboard led
            continue;
        } else {
            io_configure(io_pins[i], &input_config);
        }
    }
    // have to dotest this in sequence of the array
    for (i = 0; i < sizeof(io_pins) / sizeof(io_pins[0]); i++) {
        if (io_pins[i] == IO_TEST_LED) { // pin for nucleo board onboard led
            continue;
        }
        led_set(LED_TEST, LED_STATE_ON);
        // Wait for user to connect pull down to escape the loop
        while (io_get_input(io_pins[i]) == IO_IN_HIGH) {
            BUSY_WAIT_ms(80)
        }
        led_set(LED_TEST, LED_STATE_OFF);
        // wait for user to disconnect pulldown for pin to go HIGH again and leave the loop
        while (io_get_input(io_pins[i]) == IO_IN_LOW) {
            BUSY_WAIT_ms(80)
        }
    }
    // led flashes after all input pins are tested in order
    while (1) {
        led_set(LED_TEST, LED_STATE_ON);
        BUSY_WAIT_ms(80)

            led_set(LED_TEST, LED_STATE_OFF);
        BUSY_WAIT_ms(80)
    }
}

SUPPRESS_UNUSED
static void io_PA_8_isr(void)
{
    led_set(LED_TEST, LED_STATE_ON);
}

SUPPRESS_UNUSED
static void io_PB_3_isr(void)
{
    led_set(LED_TEST, LED_STATE_OFF);
}

SUPPRESS_UNUSED
static void test_io_interrupt(void)
{
    test_setup();
    const struct io_config input_config = { .mode = IO_MODE_INPUT,
                                            .pupd = IO_PORT_PU,
                                            .speed = IO_SPEED_LOW,
                                            .type = IO_TYPE_PP,
                                            .af = IO_AF_NONE };
    io_configure((io_e)IO_PA_8, &input_config);
    io_configure((io_e)IO_PB_3, &input_config);
    led_init();

    io_interrupt_config((io_e)IO_PB_3, io_PB_3_isr, IO_FALLING_TRIGGER, IO_EXTI_N_3);
    io_enable_interrupt(IO_EXTI_3_LINE);
    io_interrupt_config((io_e)IO_PA_8, io_PA_8_isr, IO_FALLING_TRIGGER, IO_EXTI_N_0);
    io_enable_interrupt(IO_EXTI_9_5_LINE);

    while (1)

        ;
}
SUPPRESS_UNUSED
static void test_ir_remote(void)
{
    test_setup();
    trace_init();
    ir_remote_init();
    volatile int j = 0;
    while (1) {
        TRACE("Command: %s", ir_get_cmd_str(ir_remote_get_cmd()));
        BUSY_WAIT_ms(40)
    }
}
SUPPRESS_UNUSED
static void test_pwm(void)
{
    test_setup();
    trace_init();
    pwm_init();
    const uint8_t duty_cycles[] = { 100, 75, 50, 25, 1, 0 };
    const uint16_t pwm_delay = 300;
    volatile int j;
    while (1) {
        for (uint8_t i = 0; i < ARRAY_SIZE(duty_cycles); i++) {
            TRACE("Duty cycle set to %d for %d ms", duty_cycles[i], pwm_delay);
            pwm_set_duty_cycle(PWM_TB6612FNG_LEFT_CH, duty_cycles[i]);
            pwm_set_duty_cycle(PWM_TB6612FNG_RIGHT_CH, duty_cycles[i]);
            BUSY_WAIT_ms(pwm_delay)
        }
    }
}
SUPPRESS_UNUSED
static void test_tb6612fng(void)
{
    test_setup();
    trace_init();
    tb6612fng_init();
    const tb6612fng_mode_e modes[] = { TB6612FNG_MODE_FORMARD, TB6612FNG_MODE_REVERSE,
                                       TB6612FNG_MODE_FORMARD, TB6612FNG_MODE_REVERSE };
    const uint8_t duty_cycles[] = { 100, 50, 25, 0 };
    const uint16_t pwm_delay = 1000;
    volatile int j;
    while (1) {
        for (uint8_t i = 0; i < ARRAY_SIZE(duty_cycles); i++) {
            TRACE("Duty cycle set to %d for %d ms, with mode %d", duty_cycles[i], pwm_delay,
                  modes[i]);
            tb6612fng_set_mode(TB6612FNG_LEFT, modes[i]);
            tb6612fng_set_mode(TB6612FNG_RIGHT, modes[i]);
            tb6612fng_set_pwm(TB6612FNG_LEFT, duty_cycles[i]);
            tb6612fng_set_pwm(TB6612FNG_RIGHT, duty_cycles[i]);
            BUSY_WAIT_ms(pwm_delay)
        }
    }
}
SUPPRESS_UNUSED
static void test_drive(void)
{
    test_setup();
    trace_init();
    drive_init();
    ir_remote_init();
    volatile int j = 0;
    drive_dir_e dir = DRIVE_FORWARD_DIR;
    drive_speed_e speed = DRIVE_SPEED_LOW;
    ir_cmd_e cmd;
    while (1) {
        BUSY_WAIT_ms(100);
        cmd = ir_remote_get_cmd();
        TRACE("Command: %s", ir_get_cmd_str(ir_remote_get_cmd()));

        switch (cmd) {
        case IR_CMD_0:
            drive_stop();
            continue;
            ;
        case IR_CMD_1:
            speed = DRIVE_SPEED_LOW;
            break;
        case IR_CMD_2:
            speed = DRIVE_SPEED_MEDIUM;
            break;
        case IR_CMD_3:
            speed = DRIVE_SPEED_HIGH;
            break;
        case IR_CMD_4:
            speed = DRIVE_SPEED_MAX;
            break;
        case IR_CMD_UP:
            dir = DRIVE_FORWARD_DIR;
            break;
        case IR_CMD_DOWN:
            dir = DRIVE_REVERSE_DIR;
            break;
        case IR_CMD_LEFT:
            dir = DRIVE_ROTATE_LEFT_DIR;
            break;
        case IR_CMD_RIGHT:
            dir = DRIVE_ROTATE_RIGHT_DIR;
            break;
        case IR_CMD_5:
        case IR_CMD_6:
        case IR_CMD_7:
        case IR_CMD_8:
        case IR_CMD_9:
        case IR_CMD_ASTERISK:
        case IR_CMD_POUND:
        case IR_CMD_OK:
        case IR_CMD_NONE:
            continue;
        };
        drive_set_config(dir, speed);
    }
}
SUPPRESS_UNUSED
static void test_stop_motors_assert(void)
{
    test_setup();
    trace_init();
    drive_init();
    volatile int j;
    drive_set_config(DRIVE_FORWARD_DIR, DRIVE_SPEED_MAX);
    BUSY_WAIT_ms(2000);
    ASSERT(0);
    while (0)
        ;
}
SUPPRESS_UNUSED
static void test_adc(void)
{
    test_setup();
    trace_init();
    adc_init();
    volatile int j;
    adc_channel_values adc_channel_data = { 0 };
    while (1) {
        adc_read_channel_values(adc_channel_data);
        // channel 1 -> index 0
        // using (i+1) to match actual ADC channel structure, i.e. 1-16 rather than 0-15
        for (uint8_t i = 0; i < ADC_CHANNEL_CNT; i++) {
            TRACE("ADC CHANNEL %u | VAL : %u\n", i, adc_channel_data[i]);
        }
        BUSY_WAIT_ms(300);
    };
}

SUPPRESS_UNUSED
static void test_adc_single(void)
{
    test_setup();
    trace_init();
    adc_single_init();
    volatile int j;
    uint32_t value;
    while (1) {
        adc_read_value(&value);
        TRACE("ADC CHANNEL %u | VAL : %u", 15, value);
        BUSY_WAIT_ms(500);
    };
}
SUPPRESS_UNUSED
static void test_qre1113(void)
{
    test_setup();
    trace_init();
    qre1113_init();
    volatile int j;
    struct qre1113_voltages voltages = { 0, 0, 0, 0 };
    while (1) {
        qre1113_get_voltages(&voltages);

        TRACE("LINE SENSOR FRONT RIGHT: %u", voltages.qre1113_front_right);
        TRACE("LINE SENSOR FRONT LEFT: %u", voltages.qre1113_front_left);
        TRACE("LINE SENSOR BACK RIGHT: %u", voltages.qre1113_back_right);
        TRACE("LINE SENSOR BACK LEFT: %u\n", voltages.qre1113_back_left);
        BUSY_WAIT_ms(500);
    };
}
SUPPRESS_UNUSED
static char *line_to_string(line_pos_e line)
{
    switch (line) {
    case LINE_FRONT:
        return "LINE FRONT";
        break;
    case LINE_BACK:
        return "LINE BACK";
        break;
    case LINE_LEFT:
        return "LINE LEFT";
        break;
    case LINE_RIGHT:
        return "LINE RIGHT";
        break;
    case LINE_FRONT_LEFT:
        return "LINE FRONT LEFT";
        break;
    case LINE_FRONT_RIGHT:
        return "LINE FRONT RIGHT";
        break;
    case LINE_BACK_LEFT:
        return "LINE BACK LEFT";
        break;
    case LINE_BACK_RIGHT:
        return "LINE BACK RIGHT";
        break;
    case LINE_DIAGONAL_LEFT:
        return "LINE DIAGONAL LEFT";
        break;
    case LINE_DIAGONAL_RIGHT:
        return "LINE DIAGONAL RIGHT";
        break;
    default:
        return "LINE NONE";
    }
}
SUPPRESS_UNUSED
static void test_line_detect(void)
{
    test_setup();
    trace_init();
    line_init();
    volatile int j;
    while (1) {
        TRACE("LINE POS: %s", line_to_string(get_line_position()));
        BUSY_WAIT_ms(200);
    };
}
SUPPRESS_UNUSED
static void test_i2c(void)
{
    test_setup();
    trace_init();
    i2c_init();
    volatile int j;
    uint8_t urm09_product_id = 0;
    i2c_set_slave_addr(0x11);
    // wait a bit for sensor to leave standby
    BUSY_WAIT_ms(200);

    while (1) {
        i2c_result_code_e result = i2c_read_addr8_data8(0x01, &urm09_product_id);
        if (result) {
            TRACE("I2C result error: %d", result);
        } else {
            if (urm09_product_id == 0x01) {
                TRACE("Valid URM09 PRODUCT ID (0x01): %#X", urm09_product_id);
            } else {
                TRACE("Invalid URM09 PRODUCT ID (expected 0x01): %#X", urm09_product_id);
            }
        }
        BUSY_WAIT_ms(200);
    }
}
SUPPRESS_UNUSED
static void test_urm09(void)
{
    test_setup();
    trace_init();
    urm09_init();
    volatile int j;
    uint16_t range = 0;
    urm09_set_measurement_mode(URM09_MEASURE_MODE_AUTOMATIC);
    // wait a bit for sensor
    BUSY_WAIT_ms(200);

    while (1) {
        urm09_result_e result = urm09_get_distance(&range);
        if (result) {
            TRACE("I2C result error: %d", result);
        } else {

            if (range != URM09_OUT_OF_RANGE) {
                TRACE("URM09 detected range (cm): %u", range);
            } else {
                TRACE("URM09 out of range (cm): %u", range);
            }
        }
        BUSY_WAIT_ms(200);
    }
}
SUPPRESS_UNUSED
void test_enemy(void)
{
    test_setup();
    trace_init();
    enemy_init();
    volatile int j;
    uint16_t range;
    while (1) {
        struct enemy enemy = enemy_get(&range);
        TRACE("%s %s %u", enemy_pos_to_str(enemy.position), enemy_range_to_str(enemy.range), range);
        BUSY_WAIT_ms(300);
    }
}
int main()
{
    TEST();
    ASSERT(0);
}