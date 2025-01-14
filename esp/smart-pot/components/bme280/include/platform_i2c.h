#ifndef PLATFORM_I2C_H
#define PLATFORM_I2C_H

#include <stdint.h>
#include "bme280.h" // Include the definition for BME280

void platform_i2c_init();

// Function to read data over I2C
int i2c_read(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);

// Function to write data over I2C
int i2c_write(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);

#endif // PLATFORM_I2C_H
