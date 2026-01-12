#ifndef QRE1113_H
#include <stdint.h>
// interface for the QRE1113 sensor with the ADC driver, to get the line voltages for each sensor

struct qre1113_voltages
{
    uint16_t qre1113_front_left;
    uint16_t qre1113_back_left;
    uint16_t qre1113_front_right;
    uint16_t qre1113_back_right;
};
void qre1113_init(void);
void qre1113_get_voltages(struct qre1113_voltages *voltages);

#define QRE1113_H
#endif // QRE1113_H