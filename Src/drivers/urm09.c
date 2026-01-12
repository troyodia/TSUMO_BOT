#include "urm09.h"
#include "i2c.h"
#include "assert_handler.h"
#include <stdbool.h>

#define DEFAULT_URM09_ADDR (0x11)
#define URM09_DEVICE_ID (0x01)

static bool initialized = false;

static union {
    struct
    {
        uint8_t urm09_high_order_dist;
        uint8_t urm09_low_order_dist;
    } urm09_dist_bytes;

    uint16_t measured_dist;
} urm09_measured_dist;

static urm09_result_e device_is_booted(void)
{
    uint8_t urm09_product_id = 0;
    i2c_result_code_e result = i2c_read_addr8_data8(URM09_PID_INDEX, &urm09_product_id);
    if (result) {
        return URM09_RESULT_ERROR_I2C;
    }
    return (urm09_product_id == URM09_DEVICE_ID) ? URM09_RESULT_OK : URM09_RESULT_ERROR_BOOT;
}
static urm09_result_e urm09_init_address(void)
{
    i2c_set_slave_addr(DEFAULT_URM09_ADDR);
    for (int i = 0; i < 5000; i++)
        ;
    urm09_result_e result = device_is_booted();
    if (result) {
        return result;
    }
    return URM09_RESULT_OK;
}

urm09_result_e urm09_init(void)
{
    ASSERT(!initialized);
    i2c_init();
    initialized = true;
    urm09_result_e result = urm09_init_address();
    if (result) {
        return result;
    }
    return URM09_RESULT_OK;
}
urm09_result_e urm09_set_measurement_mode(const uint8_t mode)
{
    i2c_result_code_e result = i2c_write_addr8_data8(URM09_CFG_INDEX, mode);
    if (result) {
        return URM09_RESULT_ERROR_I2C;
    }
    return URM09_RESULT_OK;
}
urm09_result_e urm09_get_distance(uint16_t *range)
{
    uint8_t dist_buf[2] = { 0 };
    urm09_result_e result =
        (urm09_result_e)i2c_read_addr8_data16(URM09_DIST_H_INDEX, (uint16_t *)dist_buf);
    if (result) {
        return URM09_RESULT_ERROR_I2C;
    }
    urm09_measured_dist.urm09_dist_bytes.urm09_high_order_dist = dist_buf[0];
    urm09_measured_dist.urm09_dist_bytes.urm09_low_order_dist = dist_buf[1];
    if (urm09_measured_dist.measured_dist == URM09_OUT_OF_RANGE) {
        *range = URM09_OUT_OF_RANGE;
    } else {
        *range = urm09_measured_dist.measured_dist;
    }
    return URM09_RESULT_OK;
}
