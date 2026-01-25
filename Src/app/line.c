#include "line.h"
#include "qre1113.h"
#include "assert_handler.h"
#include "stdbool.h"
/* threshold to distinguish between white and black platform
 * When the sensor voltage above the threshold the black platform is detected and the opposite is
 * true for detection of the white platform */
#define LINE_DETECT_VOLTAGE_THRESHOLD (3450U)
bool initialized = false;

void line_init(void)
{
    ASSERT(!initialized);
    qre1113_init();
    initialized = true;
}
line_pos_e get_line_position(void)
{
    struct qre1113_voltages voltages = { 0, 0, 0, 0 };
    qre1113_get_voltages(&voltages);
    const bool front_right_detect = voltages.qre1113_front_right < LINE_DETECT_VOLTAGE_THRESHOLD;
    const bool front_left_detect = voltages.qre1113_front_left < LINE_DETECT_VOLTAGE_THRESHOLD;
    const bool back_right_detect = voltages.qre1113_back_right < LINE_DETECT_VOLTAGE_THRESHOLD;
    const bool back_left_detect = voltages.qre1113_back_left < LINE_DETECT_VOLTAGE_THRESHOLD;
    if (front_right_detect) {
        if (front_left_detect) {
            return LINE_FRONT;
        } else if (back_right_detect) {
            return LINE_RIGHT;
        } else if (back_left_detect) {
            return LINE_DIAGONAL_RIGHT;
        } else {
            return LINE_FRONT_RIGHT;
        }
    } else if (front_left_detect) {
        if (back_left_detect) {
            return LINE_LEFT;
        } else if (back_right_detect) {
            return LINE_DIAGONAL_LEFT;
        } else {
            return LINE_FRONT_LEFT;
        }
    } else if (back_right_detect) {
        if (back_left_detect) {
            return LINE_BACK;
        } else {
            return LINE_BACK_RIGHT;
        }
    } else if (back_left_detect) {
        return LINE_BACK_LEFT;
    }
    return LINE_NONE;
}
