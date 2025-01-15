#include "bme280.h"
#include "platform_i2c.h" // Implement platform-specific I2C functions here
#include <stddef.h>  // For NULL
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



void bme280_init(BME280 *dev, uint8_t addr) {
    printf("[DEBUG] Entered bme280_init\n");

    if (dev == NULL) {
        printf("[ERROR] Device structure is NULL\n");
        return;
    }

    printf("[DEBUG] Assigning I2C address: 0x%02X\n", addr);
    dev->i2c_addr = addr;

    uint8_t id = 0;
    // Read device ID
    if (i2c_read(dev->i2c_addr, BME280_REG_ID, &id, 1) != 0) {
        printf("[ERROR] Failed to read device ID\n");
        return;
    }

    if (id != 0x60) {
        printf("[ERROR] Invalid device ID: 0x%02X (expected 0x60)\n", id);
        return;
    }
    printf("[DEBUG] Device ID: 0x%02X\n", id);

    // Reset sensor
    uint8_t reset_cmd = 0xB6;
    if (i2c_write(dev->i2c_addr, BME280_REG_RESET, &reset_cmd, 1) != 0) {
        printf("[ERROR] Failed to reset the sensor\n");
        return;
    }
    // printf("[DEBUG] Sensor reset command sent successfully\n");

    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Read calibration data
    uint8_t calib[26];
    if (i2c_read(dev->i2c_addr, BME280_CALIB_00, calib, 26) != 0) {
        printf("[ERROR] Failed to read calibration data from 0x88\n");
        return;
    }
    // printf("[DEBUG] Calibration data from 0x88 to 0xA1: ");
    // for (int i = 0; i < 26; i++) {
    //     printf("0x%02X ", calib[i]);
    // }
    // printf("\n");

    // Parse calibration data
    dev->calib_data.dig_T1 = (calib[1] << 8) | calib[0];
    dev->calib_data.dig_T2 = (calib[3] << 8) | calib[2];
    dev->calib_data.dig_T3 = (calib[5] << 8) | calib[4];
    dev->calib_data.dig_P1 = (calib[7] << 8) | calib[6];
    dev->calib_data.dig_P2 = (calib[9] << 8) | calib[8];
    dev->calib_data.dig_P3 = (calib[11] << 8) | calib[10];
    dev->calib_data.dig_P4 = (calib[13] << 8) | calib[12];
    dev->calib_data.dig_P5 = (calib[15] << 8) | calib[14];
    dev->calib_data.dig_P6 = (calib[17] << 8) | calib[16];
    dev->calib_data.dig_P7 = (calib[19] << 8) | calib[18];
    dev->calib_data.dig_P8 = (calib[21] << 8) | calib[20];
    dev->calib_data.dig_P9 = (calib[23] << 8) | calib[22];

    // Read additional humidity calibration data
    uint8_t calib_h[7];
    if (i2c_read(dev->i2c_addr, BME280_CALIB_H, calib_h, 7) != 0) {
        printf("[ERROR] Failed to read calibration data from 0xE1\n");
        return;
    }

    dev->calib_data.dig_H1 = calib[25];
    dev->calib_data.dig_H2 = (calib_h[1] << 8) | calib_h[0];
    dev->calib_data.dig_H3 = calib_h[2];
    dev->calib_data.dig_H4 = (calib_h[3] << 4) | (calib_h[4] & 0x0F);
    dev->calib_data.dig_H5 = (calib_h[5] << 4) | (calib_h[4] >> 4);
    dev->calib_data.dig_H6 = calib_h[6];


    // Set default oversampling and power mode
    uint8_t ctrl_hum = BME280_OVERSAMPLING_1X;
    if (i2c_write(dev->i2c_addr, BME280_REG_CTRL_HUM, &ctrl_hum, 1) != 0) {
        printf("[ERROR] Failed to write CTRL_HUM\n");    // Print parsed calibration data
    // printf("Calibration Data:\n");
    // printf("T1: %u, T2: %d, T3: %d\n", dev->calib_data.dig_T1, dev->calib_data.dig_T2, dev->calib_data.dig_T3);
    // printf("P1: %u, P2: %d, P3: %d, P4: %d, P5: %d\n",
    //        dev->calib_data.dig_P1, dev->calib_data.dig_P2, dev->calib_data.dig_P3,
    //        dev->calib_data.dig_P4, dev->calib_data.dig_P5);
    // printf("P6: %d, P7: %d, P8: %d, P9: %d\n",
    //        dev->calib_data.dig_P6, dev->calib_data.dig_P7,
    //        dev->calib_data.dig_P8, dev->calib_data.dig_P9);
    // printf("H1: %u, H2: %d, H3: %u, H4: %d, H5: %d, H6: %d\n",
    //        dev->calib_data.dig_H1, dev->calib_data.dig_H2, dev->calib_data.dig_H3,
    //        dev->calib_data.dig_H4, dev->calib_data.dig_H5, dev->calib_data.dig_H6);
        return;
    }
    // printf("[DEBUG] CTRL_HUM set successfully\n");

    uint8_t ctrl_meas = (BME280_OVERSAMPLING_1X << 5) | (BME280_OVERSAMPLING_1X << 2) | BME280_NORMAL_MODE;
    if (i2c_write(dev->i2c_addr, BME280_REG_CTRL_MEAS, &ctrl_meas, 1) != 0) {
        printf("[ERROR] Failed to write CTRL_MEAS\n");
        return;
    }
    // printf("[DEBUG] CTRL_MEAS set successfully\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("[DEBUG] BME280 initialization complete\n");
}


#include "platform_i2c.h"
#include "driver/i2c.h"
esp_err_t bme280_read_raw_data(BME280 *dev, int32_t *temp, int32_t *press, int32_t *hum) {
    if (dev == NULL || temp == NULL || press == NULL || hum == NULL) {
        printf("Null pointer error in bme280_read_raw_data\n");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[8];

    // Read raw sensor data from the BME280 registers
    if (i2c_read(dev->i2c_addr, BME280_REG_PRESS_MSB, data, sizeof(data)) != 0) {
        printf("I2C read error in bme280_read_raw_data\n");
        return ESP_FAIL;
    }

    // Parse raw pressure, temperature, and humidity data
    *press = (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4));
    *temp = (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4));
    *hum = (int32_t)((data[6] << 8) | data[7]);

    return ESP_OK;
}

esp_err_t bme280_read_chip_id(BME280 *dev, uint8_t *chip_id) {
    if (dev == NULL || chip_id == NULL) {
        return ESP_ERR_INVALID_ARG;  // Sprawdzenie na null
    }

    // Rejestr ID: 0xD0
    if (i2c_read(dev->i2c_addr, BME280_REG_ID, chip_id, 1) != 0) {
        printf("[ERROR] Failed to read chip ID from BME280\n");
        return ESP_FAIL;
    }

    // printf("[DEBUG] Chip ID: 0x%02X\n", *chip_id);
    return ESP_OK;
}

esp_err_t bme280_read_temp(BME280 *dev, float *temperature) {
    if (dev == NULL || temperature == NULL) {
        printf("[ERROR] Invalid arguments to bme280_read_temperature\n");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[8]; // Odczyt pełnych 8 bajtów danych
    if (i2c_read(dev->i2c_addr, BME280_REG_PRESS_MSB, data, 8) != 0) {
        printf("[ERROR] Failed to read full sensor data\n");
        return ESP_FAIL;
    }

    int32_t raw_temp = (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4));
    *temperature = bme280_compensate_temp(dev, raw_temp);
    bme280_compensate_hum(dev, (int32_t)((data[6] << 8) | data[7]));
    bme280_compensate_press(dev, (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4)));
    return ESP_OK;
}

esp_err_t bme280_read_humidity(BME280 *dev, float *humidity) {
    if (dev == NULL || humidity == NULL) {
        printf("[ERROR] Invalid arguments to bme280_read_humidity\n");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[8]; // Odczyt pełnych 8 bajtów danych
    if (i2c_read(dev->i2c_addr, BME280_REG_PRESS_MSB, data, 8) != 0) {
        printf("[ERROR] Failed to read full sensor data\n");
        return ESP_FAIL;
    }

    int32_t raw_hum = (int32_t)((data[6] << 8) | data[7]);
    bme280_compensate_temp(dev, (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4)));
    *humidity = bme280_compensate_hum(dev, raw_hum);
    bme280_compensate_press(dev, (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4)));
    return ESP_OK;
}


esp_err_t bme280_read_pressure(BME280 *dev, float *pressure) {
    if (dev == NULL || pressure == NULL) {
        printf("[ERROR] Invalid arguments to bme280_read_pressure\n");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[8]; // Odczyt pełnych 8 bajtów danych
    if (i2c_read(dev->i2c_addr, BME280_REG_PRESS_MSB, data, 8) != 0) {
        printf("[ERROR] Failed to read full sensor data\n");
        return ESP_FAIL;
    }

    int32_t raw_press = (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4));
    bme280_compensate_temp(dev, (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4)));
    bme280_compensate_hum(dev, (int32_t)((data[6] << 8) | data[7]));
    *pressure = bme280_compensate_press(dev, raw_press);
    return ESP_OK;
}



float bme280_compensate_temp(BME280 *dev, int32_t raw_temp) {
    int32_t var1, var2;
    var1 = ((((raw_temp >> 3) - ((int32_t)dev->calib_data.dig_T1 << 1))) *
            ((int32_t)dev->calib_data.dig_T2)) >>
           11;
    var2 = (((((raw_temp >> 4) - ((int32_t)dev->calib_data.dig_T1)) *
              ((raw_temp >> 4) - ((int32_t)dev->calib_data.dig_T1))) >>
             12) *
            ((int32_t)dev->calib_data.dig_T3)) >>
           14;
    dev->t_fine = var1 + var2;
    // printf("t_fine: %lu\n", dev->t_fine);

    return (float)((dev->t_fine * 5 + 128) >> 8) / 100.0f;
}


float bme280_compensate_press(BME280 *dev, int32_t raw_press) {
    int64_t var1, var2, p;
    var1 = ((int64_t)dev->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dev->calib_data.dig_P6;
    var2 = var2 + ((var1 * (int64_t)dev->calib_data.dig_P5) << 17);
    var2 = var2 + (((int64_t)dev->calib_data.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dev->calib_data.dig_P3) >> 8) +
           ((var1 * (int64_t)dev->calib_data.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dev->calib_data.dig_P1) >> 33;

    if (var1 == 0) {
        return 0;  // Avoid division by zero
    }

    p = 1048576 - raw_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dev->calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dev->calib_data.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dev->calib_data.dig_P7) << 4);

    return (float)p / 25600.0f;  // Convert Pa to hPa
}


float bme280_compensate_hum(BME280 *dev, int32_t raw_hum) {
    if (dev == NULL) {
        return 0.0f;
    }

    int32_t v_x1_u32r;
    v_x1_u32r = (dev->t_fine - ((int32_t)76800));
    v_x1_u32r = (((((raw_hum << 14) - (((int32_t)dev->calib_data.dig_H4) << 20) -
                    (((int32_t)dev->calib_data.dig_H5) * v_x1_u32r)) +
                   ((int32_t)16384)) >>
                  15) *
                 (((((((v_x1_u32r * ((int32_t)dev->calib_data.dig_H6)) >> 10) *
                      (((v_x1_u32r * ((int32_t)dev->calib_data.dig_H3)) >> 11) + ((int32_t)32768))) >>
                     10) +
                    ((int32_t)2097152)) *
                       ((int32_t)dev->calib_data.dig_H2) +
                   8192) >>
                  14));
    v_x1_u32r = (v_x1_u32r -
                 (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                   ((int32_t)dev->calib_data.dig_H1)) >>
                  4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (float)(v_x1_u32r >> 12) / 1024.0f; // Humidity in %
}

esp_err_t bme280_set_ctrl_meas(BME280 *dev, uint8_t osrs_t, uint8_t osrs_p, uint8_t mode) {
    if (dev == NULL) {
        printf("[ERROR] Device structure is NULL\n");
        return ESP_ERR_INVALID_ARG;
    }

    // Budowanie wartości rejestru ctrl_meas: osrs_t[2:0], osrs_p[2:0], mode[1:0]
    uint8_t ctrl_meas = ((osrs_t & 0x07) << 5) | ((osrs_p & 0x07) << 2) | (mode & 0x03);

    // Zapis wartości do rejestru CTRL_MEAS (0xF4)
    if (i2c_write(dev->i2c_addr, BME280_REG_CTRL_MEAS, &ctrl_meas, 1) == 0) {
        printf("[DEBUG] ctrl_meas set to 0x%02X (osrs_t: %u, osrs_p: %u, mode: %u)\n", 
               ctrl_meas, osrs_t, osrs_p, mode);
        return ESP_OK;
    } else {
        printf("[ERROR] Failed to write ctrl_meas register\n");
        return ESP_FAIL;
    }
}

esp_err_t bme280_read_ctrl_meas(BME280 *dev, uint8_t *ctrl_meas) {
    if (dev == NULL || ctrl_meas == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (i2c_read(dev->i2c_addr, BME280_REG_CTRL_MEAS, ctrl_meas, 1) == 0) {
        printf("[DEBUG] ctrl_meas read: 0x%02X\n", *ctrl_meas);
        return ESP_OK;
    } else {
        printf("[ERROR] Failed to read ctrl_meas register\n");
        return ESP_FAIL;
    }
}

esp_err_t bme280_set_ctrl_hum(BME280 *dev, uint8_t osrs_h) {
    if (dev == NULL) {
        printf("[ERROR] Device structure is NULL\n");
        return ESP_ERR_INVALID_ARG;
    }

    if (osrs_h > 0x05) {  // osrs_h values beyond 0x05 are invalid
        printf("[ERROR] Invalid humidity oversampling value: 0x%02X\n", osrs_h);
        return ESP_ERR_INVALID_ARG;
    }

    // Write to ctrl_hum register (0xF2)
    uint8_t reg_value = osrs_h & 0x07;  // Mask to ensure only lower 3 bits are used
    if (i2c_write(dev->i2c_addr, BME280_REG_CTRL_HUM, &reg_value, 1) != 0) {
        printf("[ERROR] Failed to write to CTRL_HUM register\n");
        return ESP_FAIL;
    }

    // Ensure the changes take effect by writing again to ctrl_meas
    uint8_t ctrl_meas = (BME280_OVERSAMPLING_1X << 5) | (BME280_OVERSAMPLING_1X << 2) | BME280_NORMAL_MODE;
    if (i2c_write(dev->i2c_addr, BME280_REG_CTRL_MEAS, &ctrl_meas, 1) != 0) {
        printf("[ERROR] Failed to re-write CTRL_MEAS after CTRL_HUM\n");
        return ESP_FAIL;
    }

    // printf("[DEBUG] CTRL_HUM set to 0x%02X\n", reg_value);

    return ESP_OK;
}

esp_err_t bme280_read_ctrl_hum(BME280 *dev, uint8_t *osrs_h) {
    if (dev == NULL || osrs_h == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t ctrl_hum = 0;
    if (i2c_read(dev->i2c_addr, BME280_REG_CTRL_HUM, &ctrl_hum, 1) == 0) {
        *osrs_h = ctrl_hum & 0x07; // Odczytaj tylko 3 najmłodsze bity osrs_h[2:0]
        printf("[DEBUG] ctrl_hum read: 0x%02X (osrs_h: 0x%02X)\n", ctrl_hum, *osrs_h);
        return ESP_OK;
    } else {
        printf("[ERROR] Failed to read ctrl_hum register\n");
        return ESP_FAIL;
    }
}

esp_err_t bme280_set_config(BME280 *dev, uint8_t t_sb, uint8_t filter, uint8_t spi3w_en) {
    if (dev == NULL) {
        printf("[ERROR] Device structure is NULL\n");
        return ESP_ERR_INVALID_ARG;
    }

    // Mask the input parameters to ensure they are within valid ranges
    t_sb &= 0x07;       // t_sb[2:0]: 3 bits
    filter &= 0x07;     // filter[2:0]: 3 bits
    spi3w_en &= 0x01;   // spi3w_en[0]: 1 bit

    // Build the config register value
    uint8_t config_value = (t_sb << 5) | (filter << 2) | spi3w_en;

    // Write to the config register (0xF5)
    if (i2c_write(dev->i2c_addr, BME280_REG_CONFIG, &config_value, 1) == 0) {
        printf("[DEBUG] Config register set to 0x%02X (t_sb: %u, filter: %u, spi3w_en: %u)\n",
               config_value, t_sb, filter, spi3w_en);
        return ESP_OK;
    } else {
        printf("[ERROR] Failed to write config register\n");
        return ESP_FAIL;
    }
}

esp_err_t bme280_read_config(BME280 *dev, uint8_t *config) {
    if (dev == NULL || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Read the config register (0xF5)
    if (i2c_read(dev->i2c_addr, BME280_REG_CONFIG, config, 1) == 0) {
        printf("[DEBUG] Config register read: 0x%02X\n", *config);
        return ESP_OK;
    } else {
        printf("[ERROR] Failed to read config register\n");
        return ESP_FAIL;
    }
}

esp_err_t bme280_read_status(BME280 *dev, uint8_t *status) {
    if (dev == NULL || status == NULL) {
        printf("[ERROR] Invalid arguments to bme280_read_status\n");
        return ESP_ERR_INVALID_ARG;
    }

    if (i2c_read(dev->i2c_addr, BME280_REG_STATUS, status, 1) == 0) {
        printf("[DEBUG] Status register read: 0x%02X\n", *status);
        return ESP_OK;
    } else {
        printf("[ERROR] Failed to read status register\n");
        return ESP_FAIL;
    }
}

void bme280_interpret_status(uint8_t status, uint8_t *measuring, uint8_t *im_update) {
    if (measuring != NULL) {
        *measuring = (status >> 3) & 0x01; // Extract the measuring bit (bit 3)
    }
    if (im_update != NULL) {
        *im_update = status & 0x01; // Extract the im_update bit (bit 0)
    }
}

esp_err_t bme280_reset(BME280 *dev) {
    if (dev == NULL) {
        printf("[ERROR] Device structure is NULL\n");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t reset_value = 0xB6;

    // Write the reset value to the RESET register (0xE0)
    if (i2c_write(dev->i2c_addr, BME280_REG_RESET, &reset_value, 1) == 0) {
        printf("[DEBUG] Device reset command sent (0xB6)\n");
        vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to allow reset to complete
        return ESP_OK;
    } else {
        printf("[ERROR] Failed to send reset command\n");
        return ESP_FAIL;
    }
}
