#include "drive.h"
#include "tb6612fng.h"
#include "assert_handler.h"
#include "defines.h"
#include <stdbool.h>
#include <assert.h>

static bool intialized = false;

struct drive_speed_duty_cycle
{
    uint8_t left;
    uint8_t right;
};
// only add the first in the pairs of directions (e.g. FORARD and REVERSE) to array to save space
// works based on extracting the LSB of the dir
#define DRIVE_BASIC_DIRECTION(dir) (dir - MOD_2(dir))
static_assert(DRIVE_FORWARD_DIR == DRIVE_BASIC_DIRECTION(DRIVE_REVERSE_DIR));
static const struct drive_speed_duty_cycle drive_basic_speeds[][4] = {
    [DRIVE_FORWARD_DIR] = {
        [DRIVE_SPEED_LOW] = { .left = 25, .right = 25 },
        [DRIVE_SPEED_MEDIUM] = { .left = 45, .right = 45 },
        [DRIVE_SPEED_HIGH] = { .left = 55, .right = 55 },
        [DRIVE_SPEED_MAX] = { .left = 100, .right = 100 },
    },
    [DRIVE_ROTATE_LEFT_DIR] = {
        [DRIVE_SPEED_LOW] = { .left = -25, .right = 25 },
        [DRIVE_SPEED_MEDIUM] = { .left = -45, .right = 45 },
        [DRIVE_SPEED_HIGH] = { .left = -55, .right = 55 },
        [DRIVE_SPEED_MAX] = { .left = -100, .right = 100 },
    },
    [DRIVE_ARCTURN_SHARP_LEFT_DIR] = {
        [DRIVE_SPEED_LOW] = { .left = -10, .right = 25 },
        [DRIVE_SPEED_MEDIUM] = { .left = -10, .right = 45 },
        [DRIVE_SPEED_HIGH] = { .left = -25, .right = 75 },
        [DRIVE_SPEED_MAX] = { .left = -20, .right = 100 },
    },
    [DRIVE_ARCTURN_MID_LEFT_DIR] = {
        [DRIVE_SPEED_LOW] = { .left = 15, .right = 25 },
        [DRIVE_SPEED_MEDIUM] = { .left = 25, .right = 45 },
        [DRIVE_SPEED_HIGH] = { .left = 35, .right = 75 },
        [DRIVE_SPEED_MAX] = { .left = 50, .right = 100 },
    },
    [DRIVE_ARCTURN_WIDE_LEFT_DIR] = {
        [DRIVE_SPEED_LOW] = { .left = 20, .right = 25 },
        [DRIVE_SPEED_MEDIUM] = { .left = 40, .right = 45 },
        [DRIVE_SPEED_HIGH] = { .left = 55, .right = 75 },
        [DRIVE_SPEED_MAX] = { .left = 90, .right = 100 },
    },

};
void drive_init(void)
{
    ASSERT(!intialized);
    tb6612fng_init();
    intialized = true;
}
void drive_stop(void)
{
    tb6612fng_set_mode(TB6612FNG_LEFT, TB6612FNG_MODE_STOP);
    tb6612fng_set_mode(TB6612FNG_RIGHT, TB6612FNG_MODE_STOP);
    tb6612fng_set_pwm(TB6612FNG_LEFT, 0);
    tb6612fng_set_pwm(TB6612FNG_RIGHT, 0);
}
static void drive_invert_speed(int8_t *left_speed, int8_t *right_speed)
{

    if (*left_speed == *right_speed) {
        // the direction passed was REVERSE so you want to inverese the FORWAD direction
        *left_speed = -*left_speed;
        *right_speed = -*left_speed;
    } else {
        int8_t const temp = *left_speed;
        *left_speed = *right_speed;
        *right_speed = temp;
    }
}
void drive_set_config(drive_dir_e dir, drive_speed_e speed)
{
    drive_dir_e basic_dir = DRIVE_BASIC_DIRECTION(dir);
    const struct drive_speed_duty_cycle drive_speeds = drive_basic_speeds[basic_dir][speed];
    int8_t left_speed = drive_speeds.left;
    int8_t right_speed = drive_speeds.right;
    if (dir != basic_dir) {
        drive_invert_speed(&left_speed, &right_speed);
    }
    ASSERT((left_speed != 0) && (right_speed != 0));
    const tb6612fng_mode_e mode_left =
        left_speed < 0 ? TB6612FNG_MODE_REVERSE : TB6612FNG_MODE_FORMARD;
    tb6612fng_set_mode(TB6612FNG_LEFT, mode_left);
    const tb6612fng_mode_e mode_right =
        right_speed < 0 ? TB6612FNG_MODE_REVERSE : TB6612FNG_MODE_FORMARD;
    tb6612fng_set_mode(TB6612FNG_RIGHT, mode_right);

    tb6612fng_set_pwm(TB6612FNG_LEFT, ABS(left_speed));
    tb6612fng_set_pwm(TB6612FNG_RIGHT, ABS(right_speed));
}