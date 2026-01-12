#include "enemy.h"
#include "assert_handler.h"
#include "urm09.h"
#include "trace.h"
#define RANGE_MAX_DETECTION_THRESHOLD (60U) // cm
#define RANGE_MID_THRESHOLD (20U) // cm
#define RANGE_CLOSE_THRESHOLD (10U) // cm
#define INVALID_RANGE (UINT16_MAX)

static bool initialized = false;
void enemy_init(void)
{
    ASSERT(!initialized);
    urm09_result_e result = urm09_init();
    if (result) {
        TRACE("URM09 Init Error %u", result);
        return;
    }
    result = urm09_set_measurement_mode(URM09_MEASURE_MODE_AUTOMATIC);
    if (result) {
        TRACE("URM09 Set Measurment Mode Error %u", result);
        return;
    }
    // wait a bit for sensor
    for (int i = 0; i < 1000; i++)
        ;
    initialized = true;
}
struct enemy enemy_get(uint16_t *range_test)
{
    struct enemy enemy = { .position = ENEMY_POS_NONE, .range = ENEMY_RANGE_NONE };
    uint16_t range_front;
    urm09_result_e result = urm09_get_distance(&range_front);
    *range_test = range_front;
    if (result) {
        TRACE("URM09 Range Detection Error %u", result);
        return enemy;
    }
    const bool front_detection = range_front < RANGE_MAX_DETECTION_THRESHOLD;
    uint16_t range = INVALID_RANGE;
    if (front_detection) {
        enemy.position = ENEMY_POS_FRONT;
        range = range_front;
    }
    if (range == INVALID_RANGE) {
        return enemy;
    }
    if (range < RANGE_CLOSE_THRESHOLD) {
        enemy.range = ENEMY_RANGE_CLOSE;
    } else if (range < RANGE_MID_THRESHOLD) {
        enemy.range = ENEMY_RANGE_MID;
    } else {
        enemy.range = ENEMY_RANGE_FAR;
    }
    return enemy;
}

bool enemy_detected(const struct enemy *enemy)
{
    return enemy->position != ENEMY_POS_NONE;
}
bool enemy_at_front(const struct enemy *enemy)
{
    return enemy->position == ENEMY_POS_FRONT;
}

const char *enemy_pos_to_str(enemy_pos_e pos)
{
    switch (pos) {
    case ENEMY_POS_NONE:
        return "NONE";
        break;
    case ENEMY_POS_FRONT:
        return "FRONT";
        break;
    }
    return "";
}
const char *enemy_range_to_str(enemy_range_e range)
{
    switch (range) {
    case ENEMY_RANGE_NONE:
        return "NONE";
        break;
    case ENEMY_RANGE_CLOSE:
        return "ENEMY_CLOSE";
        break;
    case ENEMY_RANGE_MID:
        return "ENEMY_MID";
        break;
    case ENEMY_RANGE_FAR:
        return "ENEMY_FAR";
        break;
    }
    return "";
}