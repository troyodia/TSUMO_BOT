#include "i2c.h"
#include "io.h"
#include "stm32l4xx.h"
#include "assert_handler.h"
#include <stdbool.h>
#define I2C_DEFAULT_SLAVE_ADDR (0x29)
#define RETRY_TIMEOUT (UINT16_MAX)
static const struct io_config i2c_config = { .mode = IO_MODE_ALTFN,
                                             .pupd = IO_NO_PUPD,
                                             .speed = IO_SPEED_VERY_HIGH,
                                             .type = IO_TYPE_OD,
                                             .af = IO_AF_4 };
static bool intitialized = false;
static uint8_t vl53l0x_slave_address = 0;
void i2c_init(void)
{
    ASSERT(!intitialized);
    // check that pin config is correct
    struct io_config current_config;
    io_get_io_config(IO_I2C_SCL, &current_config);
    ASSERT(io_compare_io_config(&current_config, &i2c_config));
    io_get_io_config(IO_I2C_SDA, &current_config);
    ASSERT(io_compare_io_config(&current_config, &i2c_config));

    RCC->APB1ENR1 |= 0x1 << 23;
    I2C3->CR1 &= ~0x1; // clears PE in CR1 reg to being intitialization
    I2C3->TIMINGR = 0x10D19CE4; // TIMINGR generated using stm32cubemx and default I2C speed
                                // settings i.e. standard mode
    I2C3->CR1 &= ~(0x1 << 17); // clear the bit to enable clock stretching
    I2C3->CR1 |= 0x1; // enables PE in CR1 reg to being intitialization

    intitialized = true;
}
void i2c_set_slave_addr(uint8_t addr)
{
    vl53l0x_slave_address = addr;
}
static inline void i2c_set_start_condition(void)
{
    I2C3->CR2 |= (0x1 << 13); // set start condition
}
static inline void i2c_send_tx_byte(uint8_t byte)
{
    I2C3->TXDR = byte;
}
static inline uint8_t i2c_recieve_rx_byte(void)
{
    return I2C3->RXDR;
}

static void i2c_configure(uint8_t data_size, uint8_t addr_size, i2c_state_e state)
{
    I2C3->CR2 &= ~0x3FF; // clear the slave address
    I2C3->CR2 |= vl53l0x_slave_address << 1; // write the slave address
    I2C3->CR2 &= ~(0xFF << 16); // clear NBYTES
    uint16_t size;
    switch (state) {
    case I2C_SEND:
        size = data_size + addr_size;
        break;

    default:
        size = data_size;
        break;
    }
    I2C3->CR2 |=
        size << 16; // set NBYTES number of bytes to be sent, reg of slave to be written to and data
    I2C3->CR2 |= (0x1 << 25); // set AUTOEND mode
}
static i2c_result_code_e i2c_tx_flag_wait(void)
{
    uint16_t retries = RETRY_TIMEOUT;

    // wait for TXDR to be empty
    while (!(I2C3->ISR & 0x1 << 1) && --retries)
        ;
    if (I2C3->ISR & 0x1 << 4) { // check for a NACK after transsimision
        I2C3->ICR |= 0x1 << 4; // clear NACKCF bit
        return I2C_RESULT_ERROR_TX;
    }
    if (retries == 0) {
        return I2C_RESULT_ERROR_TIMEOUT;
    }
    return I2C_RESULT_OK;
}
static i2c_result_code_e i2c_rx_flag_wait(void)
{
    uint16_t retries = RETRY_TIMEOUT;

    // wait for RXDR to be empty
    while (!(I2C3->ISR & 0x1 << 2) && --retries)
        ;
    if (I2C3->ISR & 0x1 << 4) { // check for a NACK after transsimision
        I2C3->ICR |= 0x1 << 4; // clear NACKCF bit
        return I2C_RESULT_ERROR_RX;
    }
    if (retries == 0) {
        return I2C_RESULT_ERROR_TIMEOUT;
    }
    return I2C_RESULT_OK;
}
static i2c_result_code_e i2c_start_tx_transfer(const uint8_t *vl53l0x_memory_addr,
                                               uint8_t addr_size)
{
    I2C3->CR2 &= ~(0x1 << 10); // set to write direction
    i2c_set_start_condition();

    // address could be 8, 16 or 32 bytes
    for (uint8_t i = 0; i < addr_size; i++) {
        i2c_result_code_e result = i2c_tx_flag_wait();
        if (result) {
            return result;
        }
        i2c_send_tx_byte(vl53l0x_memory_addr[i]);
    }
    return I2C_RESULT_OK; // no errors and o0 in enum so wont trigger if statment for return
}
static i2c_result_code_e i2c_start_rx_transfer(const uint8_t *vl53l0x_memory_addr,
                                               uint8_t addr_size)
{
    /* Not using AUTOEND here because it would generate a STOP after sending the address to be read
     * Using Manual mode, (AUTOEND = 0) allows for no automatic generation of STOP
     * so you can send the address and then restart I2C to read from the sensor*/

    I2C3->CR2 = 0;
    // I2C3->CR2 &= ~0x3FF; // clear the slave address
    I2C3->CR2 |= vl53l0x_slave_address << 1; // write the slave address
    // I2C3->CR2 &= ~(0xFF << 16); // clear NBYTES

    I2C3->CR2 |= addr_size
        << 16; // set NBYTES number of bytes to be sent, reg of slave to be written to and data
    I2C3->CR2 &= ~(0x1 << 10); // set to write direction
    i2c_set_start_condition();

    // address could be 8, 16 or 32 bytes
    for (uint8_t i = 0; i < addr_size; i++) {
        i2c_result_code_e result = i2c_tx_flag_wait();
        if (result) {
            return result;
        }
        i2c_send_tx_byte(vl53l0x_memory_addr[i]);
    }
    // ensure address transfer completed succefully before starting the next START condition for
    // reading bytes
    while (!(I2C3->ISR & (0x1 << 6)))
        ;
    return I2C_RESULT_OK; // no errors and 0 in enum so wont trigger if statment for return
}
static void i2c_start_rx_byte_read(void)
{
    I2C3->CR2 |= (0x1 << 10); // set to read direction
    i2c_set_start_condition();
}
i2c_result_code_e i2c_read(const uint8_t *vl53l0x_memory_addr, uint8_t addr_size, uint8_t *data,
                           uint8_t data_size)
{
    ASSERT(vl53l0x_memory_addr);
    ASSERT(addr_size > 0);
    ASSERT(data);
    ASSERT(data_size > 0);
    // check if bus is busy
    while ((I2C3->ISR & 0x1 << 15))
        ;
    i2c_result_code_e result = i2c_start_rx_transfer(vl53l0x_memory_addr, addr_size);
    if (result) {
        return result;
    }

    i2c_configure(data_size, addr_size, I2C_RECEIVE);
    i2c_start_rx_byte_read(); // restart i2c to read bytes from slave

    // Data is received MSB first, in STM32 the LSB is at 0
    // so put the MSB at the data_size - 1 pos
    // cppcheck-suppress unsignedLessThanZero
    for (uint16_t i = data_size - 1; 0 >= i; i--) {
        result = i2c_rx_flag_wait();
        if (result) {
            return result;
        }
        data[i] = i2c_recieve_rx_byte();
    }
    // wait for STOP detection to be set after NBYTES is completed
    while (!(I2C3->ISR & (0x1 << 5)))
        ;
    I2C3->ICR |= 0x1 << 5; // clear the STOP detection flag in ISR
    return result;
};
i2c_result_code_e i2c_write(const uint8_t *vl53l0x_memory_addr, uint8_t addr_size,
                            const uint8_t *data, uint8_t data_size)
{
    ASSERT(vl53l0x_memory_addr);
    ASSERT(addr_size > 0);
    ASSERT(data);
    ASSERT(data_size > 0);
    while ((I2C3->ISR & 0x1 << 15))
        ; // check if bus is busy
    i2c_configure(data_size, addr_size, I2C_SEND);
    // start transmission and send vl53l9x memory location been written to
    i2c_result_code_e result = i2c_start_tx_transfer(vl53l0x_memory_addr, addr_size);
    if (result) {
        return result;
    }

    // sending data from MSB to LSB expected by vl53l0x
    for (uint16_t i = 0; i < data_size; i++) {
        result = i2c_tx_flag_wait();
        if (result) {
            return result;
        }
        i2c_send_tx_byte(data[i]);
    } // wait for STOP detection to be set after NBYTES is completed
    while (!(I2C3->ISR & (0x1 << 5)))
        ;
    I2C3->ICR |= 0x1 << 5; // clear the STOP detection flag in ISR
    return result;
};
i2c_result_code_e i2c_read_addr8_data8(uint8_t addr, uint8_t *data)
{
    return i2c_read(&addr, 1, data, 1); // 1 -> 1 byte
}
i2c_result_code_e i2c_read_addr8_data16(uint8_t addr, uint16_t *data)
{
    return i2c_read(&addr, 1, (uint8_t *)data, 2);
}
i2c_result_code_e i2c_read_addr8_data32(uint8_t addr, uint32_t *data)
{
    return i2c_read(&addr, 1, (uint8_t *)data, 4);
}
i2c_result_code_e i2c_write_addr8_data8(uint8_t addr, const uint8_t *data)
{
    return i2c_write(&addr, 1, data, 1);
}