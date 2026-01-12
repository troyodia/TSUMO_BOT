#ifndef URM09_H
#define URM09_H
#include <stdint.h>
#define URM09_MEASURE_MODE_AUTOMATIC (0x80U) ///< automatic mode
#define URM09_MEASURE_MODE_PASSIVE (0x00U) ///< passive mode
#define URM09_OUT_OF_RANGE (65535U) ///< passive mode
typedef enum
{
    URM09_RESULT_OK,
    URM09_RESULT_ERROR_I2C,
    URM09_RESULT_ERROR_BOOT,
} urm09_result_e;
typedef enum
{
    URM09_SLAVE_ADDR_INDEX = 0,
    URM09_PID_INDEX, /* Product Id */
    URM09_VERSION_INDEX,
    URM09_DIST_H_INDEX, /**< High distance eight digits */
    URM09_DIST_L_INDEX, /**< Low  distance eight digits */
    URM09_TEMP_H_INDEX, /**< High temperature eight digits */
    URM09_TEMP_L_INDEX, /**< Low  temperature eight digits */
    URM09_CFG_INDEX,
    URM09_CMD_INDEX,
    URM09_REG_NUM
} eRegister_t;

urm09_result_e urm09_init(void);
urm09_result_e urm09_set_measurement_mode(const uint8_t mode);
urm09_result_e urm09_get_distance(uint16_t *range);

#endif // URM09_H