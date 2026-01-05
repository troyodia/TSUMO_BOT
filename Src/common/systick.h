#ifndef SYSTICK_H
#define SYSTICK_H
/* STM32 systick implementation for the millis function*/
#include <stdint.h>
void systick_init(void);
uint32_t systick_millis(void);
#endif // SYSTICK_H