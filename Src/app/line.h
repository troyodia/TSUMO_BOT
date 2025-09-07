#ifndef LINE_H
#define LINE_H
// application layer for QRE1113 line sensor to detect a white line
typedef enum
{
    LINE_NONE,
    LINE_FRONT,
    LINE_BACK,
    LINE_LEFT,
    LINE_RIGHT,
    LINE_FRONT_LEFT,
    LINE_FRONT_RIGHT,
    LINE_BACK_LEFT,
    LINE_BACK_RIGHT,
    LINE_DIAGONAL_LEFT,
    LINE_DIAGONAL_RIGHT,
} line_pos_e;

void line_init(void);
line_pos_e get_line_position(void);
#endif // LINE_H