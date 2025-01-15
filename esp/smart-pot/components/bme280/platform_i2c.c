#include "platform_i2c.h"
#include "driver/i2c.h"
#include <stdio.h>

// Define I2C parameters (adjust according to your setup)
#define I2C_MASTER_SCL_IO           22  // GPIO number for I2C clock
#define I2C_MASTER_SDA_IO           21  // GPIO number for I2C data
#define I2C_MASTER_FREQ_HZ          100000 // I2C clock frequency (100kHz)
#define I2C_MASTER_PORT             I2C_NUM_0 // I2C port number
#define I2C_MASTER_TX_BUF_DISABLE   0   // Disable TX buffer
#define I2C_MASTER_RX_BUF_DISABLE   0   // Disable RX buffer

// Initialize I2C hardware

void platform_i2c_init() {
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,   // Set the GPIO for SDA
        .scl_io_num = GPIO_NUM_22,   // Set the GPIO for SCL
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000  // Set clock speed to 100kHz
    };
    i2c_param_config(I2C_NUM_0, &config);
    i2c_driver_install(I2C_NUM_0, config.mode, 0, 0, 0); // Install the driver
}


int i2c_read(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len) {
    esp_err_t res = i2c_master_write_read_device(I2C_NUM_0, addr, &reg, 1, data, len, 1000 / portTICK_PERIOD_MS);
    return res;
}


// Function to write data over I2C
int i2c_write(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // Send the slave address with write flag
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);

    // Send the register address to write to
    i2c_master_write_byte(cmd, reg, true);

    // Write the data buffer
    i2c_master_write(cmd, data, len, true);

    // Send stop signal
    i2c_master_stop(cmd);
    int ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        printf("I2C write failed (addr=0x%02X, reg=0x%02X).\n", addr, reg);
        return -1;
    }

    return 0; 
}
