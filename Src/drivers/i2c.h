#ifndef I2C_H
#define I2C_H
#include <stdint.h>
// driver for the URM09 sensor used to detect an enemy through polling
typedef enum
{
    I2C_SEND,
    I2C_RECEIVE
} i2c_state_e;
typedef enum
{
    I2C_RESULT_OK,
    I2C_RESULT_ERROR_START,
    I2C_RESULT_ERROR_TX,
    I2C_RESULT_ERROR_RX,
    I2C_RESULT_ERROR_STOP,
    I2C_RESULT_ERROR_TIMEOUT,
} i2c_result_code_e;

void i2c_init(void);
void i2c_set_slave_addr(
    uint8_t addr); // to switch device i2c address when writing to one of the sensors
// passing the addr as a byte array because the addreess could be 8, 16 0r 32 bits
// the functions send data from MSB to LSB expected by vl53l0x

i2c_result_code_e i2c_read(const uint8_t *urm09_memory_addr, uint8_t addr_size, uint8_t *data,
                           uint8_t data_size);

i2c_result_code_e i2c_write(const uint8_t *urm09_memory_addr, uint8_t addr_size,
                            const uint8_t *data, uint8_t data_size);

i2c_result_code_e i2c_read_addr8_data8(uint8_t addr, uint8_t *data);
i2c_result_code_e i2c_read_addr8_data16(uint8_t addr, uint16_t *data);
i2c_result_code_e i2c_read_addr8_data32(uint8_t addr, uint32_t *data);
i2c_result_code_e i2c_write_addr8_data8(uint8_t addr, const uint8_t data);
#endif // I2C_H