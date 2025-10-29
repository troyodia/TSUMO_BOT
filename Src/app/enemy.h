#ifndef ENEMY_H
#define ENEMY_H
#include <stdbool.h>
#include <stdint.h>
/*Software layer for easy interface with the URM09 sensor for enemy detection*/
typedef enum
{
    ENEMY_POS_NONE,
    ENEMY_POS_FRONT,
} enemy_pos_e;

typedef enum
{
    ENEMY_RANGE_NONE,
    ENEMY_RANGE_CLOSE,
    ENEMY_RANGE_MID,
    ENEMY_RANGE_FAR,
} enemy_range_e;

struct enemy
{
    enemy_pos_e position;
    enemy_range_e range;
};
void enemy_init(void);
struct enemy enemy_get(uint16_t *range_test);

bool enemy_detected(const struct enemy *enemy);
bool enemy_at_front(const struct enemy *enemy);

const char *enemy_pos_to_str(enemy_pos_e pos);
const char *enemy_range_to_str(enemy_range_e range);
#endif // ENEMY_H