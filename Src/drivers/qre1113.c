#include "qre1113.h"
#include "adc.h"
#include "io.h"
#include "defines.h"
#include "assert_handler.h"
#include <stdbool.h>

static bool initialized = false;

void qre1113_init(void)
{
    ASSERT(!initialized);
    adc_init();
    initialized = true;
}
void qre1113_get_voltages(struct qre1113_voltages *voltages)
{
    adc_channel_values adc_qre1113_voltages;

    adc_read_channel_values(adc_qre1113_voltages);
    voltages->qre1113_front_left = adc_qre1113_voltages[io_adc_idx(IO_PA_0)];
    voltages->qre1113_back_left = adc_qre1113_voltages[io_adc_idx(IO_PA_1)];
    voltages->qre1113_front_right = adc_qre1113_voltages[io_adc_idx(IO_PA_4)];
    voltages->qre1113_back_right = adc_qre1113_voltages[io_adc_idx(IO_PB_0)];
}