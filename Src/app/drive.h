#ifndef DRIVE_H
#define DRIVE_H
// Drive interface to allow for easy to use application code in controlling the motors
typedef enum
{
    DRIVE_FORWARD_DIR,
    DRIVE_REVERSE_DIR,
    DRIVE_ROTATE_LEFT_DIR,
    DRIVE_ROTATE_RIGHT_DIR,
    DRIVE_ARCTURN_SHARP_LEFT_DIR,
    DRIVE_ARCTURN_SHARP_RIGHT_DIR,
    DRIVE_ARCTURN_MID_LEFT_DIR,
    DRIVE_ARCTURN_MID_RIGHT_DIR,
    DRIVE_ARCTURN_WIDE_LEFT_DIR,
    DRIVE_ARCTURN_WIDE_RIGHT_DIR,
} drive_dir_e;

typedef enum
{
    DRIVE_SPEED_LOW,
    DRIVE_SPEED_MEDIUM,
    DRIVE_SPEED_HIGH,
    DRIVE_SPEED_MAX
} drive_speed_e;

void drive_init(void);
void drive_stop(void);
void drive_set_config(drive_dir_e dir, drive_speed_e speed);
#endif // DRIVE_H