#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include "esp_err.h"
#define BME280_I2C_ADDR_0  0x76
#define BME280_I2C_ADDR_1  0x77

// Register addresses
#define BME280_REG_ID            0xD0
#define BME280_REG_RESET         0xE0
#define BME280_REG_CTRL_HUM      0xF2
#define BME280_REG_STATUS        0xF3
#define BME280_REG_CTRL_MEAS     0xF4
#define BME280_REG_CONFIG        0xF5
#define BME280_REG_PRESS_MSB     0xF7

// Calibration register addresses
#define BME280_CALIB_00          0x88
#define BME280_CALIB_26          0xE1

// Oversampling settings
#define BME280_OVERSAMPLING_1X   0x01
#define BME280_OVERSAMPLING_2X   0x02
#define BME280_OVERSAMPLING_4X   0x03
#define BME280_CALIB_H 0xE1  // Start address of humidity calibration data

// Power modes
#define BME280_SLEEP_MODE        0x00
#define BME280_FORCED_MODE       0x01
#define BME280_NORMAL_MODE       0x03
/****************************************************/


// Calibration register addresses
#define BME280_CALIB_00          0x88
#define BME280_CALIB_26          0xE1

// Device calibration data structure
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t  dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} BME280_CalibData;

// Device structure
typedef struct {
    uint8_t i2c_addr;               // I2C address
    int32_t t_fine;                 // Fine temperature value for compensation
    BME280_CalibData calib_data;    // Calibration data
} BME280;

// Function prototypes

/**
 * @brief Initialize the BME280 device.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param addr I2C address of the sensor.
 */
void bme280_init(BME280 *dev, uint8_t addr);

/**
 * @brief Read raw temperature, pressure, and humidity data. Then apply compensation formulas.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param temp Pointer to store raw temperature data.
 * @param press Pointer to store raw pressure data.
 * @param hum Pointer to store raw humidity data.
 */
esp_err_t bme280_read_raw_data(BME280 *dev, int32_t *temp, int32_t *press, int32_t *hum);

/**
 * @brief Read and compensate the temperature data from the BME280 sensor.
 *
 * This function reads raw temperature data from the sensor and applies the
 * compensation formula to return the actual temperature in degrees Celsius.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param temperature Pointer to store the compensated temperature value (in °C).
 *
 * @return
 *      - ESP_OK: Success.
 *      - ESP_ERR_INVALID_ARG: Invalid arguments.
 *      - ESP_FAIL: Failed to read the sensor.
 */
esp_err_t bme280_read_temp(BME280 *dev, float *temperature);

/**
 * @brief Read and compensate the humidity data from the BME280 sensor.
 *
 * This function reads raw humidity data from the sensor and applies the
 * compensation formula to return the actual relative humidity in percentage.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param humidity Pointer to store the compensated humidity value (in %).
 *
 * @return
 *      - ESP_OK: Success.
 *      - ESP_ERR_INVALID_ARG: Invalid arguments.
 *      - ESP_FAIL: Failed to read the sensor.
 */
esp_err_t bme280_read_humidity(BME280 *dev, float *humidity);

/**
 * @brief Read and compensate the pressure data from the BME280 sensor.
 *
 * This function reads raw pressure data from the sensor and applies the
 * compensation formula to return the actual pressure in hectopascals (hPa).
 *
 * @param dev Pointer to the BME280 device structure.
 * @param pressure Pointer to store the compensated pressure value (in hPa).
 *
 * @return
 *      - ESP_OK: Success.
 *      - ESP_ERR_INVALID_ARG: Invalid arguments.
 *      - ESP_FAIL: Failed to read the sensor.
 */
esp_err_t bme280_read_pressure(BME280 *dev, float *pressure);

/**
 * @brief Read the chip ID from the BME280 sensor.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param chip_id Pointer to a variable where the chip ID will be stored.
 * @return esp_err_t ESP_OK if successful, ESP_FAIL otherwise.
 */
esp_err_t bme280_read_chip_id(BME280 *dev, uint8_t *chip_id);

/**
 * @brief Compensate raw temperature data and return in degrees Celsius.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param raw_temp Raw temperature data from the sensor.
 * @return Compensated temperature in degrees Celsius.
 */
float bme280_compensate_temp(BME280 *dev, int32_t raw_temp);

/**
 * @brief Compensate raw pressure data and return in hPa.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param raw_press Raw pressure data from the sensor.
 * @return Compensated pressure in hPa.
 */
float bme280_compensate_press(BME280 *dev, int32_t raw_press);

/**
 * @brief Compensate raw humidity data and return in % relative humidity.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param raw_hum Raw humidity data from the sensor.
 * @return Compensated relative humidity in %.
 */
float bme280_compensate_hum(BME280 *dev, int32_t raw_hum);

/**
 * @brief Configure humidity oversampling in BME280.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param osrs_h Oversampling setting for humidity (0x00 to 0x05).
 * @return esp_err_t Returns ESP_OK on success, or ESP_FAIL on failure.
 */
esp_err_t bme280_set_ctrl_hum(BME280 *dev, uint8_t osrs_h);

/**
 * @brief Set the ctrl_meas register (oversampling and mode settings).
 *
 * @param dev Pointer to the BME280 device structure.
 * @param osrs_t Oversampling setting for temperature (3 bits).
 * @param osrs_p Oversampling setting for pressure (3 bits).
 * @param mode Mode setting (2 bits: Sleep, Forced, or Normal).
 * @return esp_err_t ESP_OK if successful, ESP_FAIL otherwise.
 */
esp_err_t bme280_set_ctrl_meas(BME280 *dev, uint8_t osrs_t, uint8_t osrs_p, uint8_t mode);

/**
 * @brief Read the current value of the ctrl_meas register.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param ctrl_meas Pointer to store the current value of ctrl_meas.
 * @return esp_err_t ESP_OK if successful, ESP_FAIL otherwise.
 */
esp_err_t bme280_read_ctrl_meas(BME280 *dev, uint8_t *ctrl_meas);

/**
 * @brief Read the current value of the ctrl_hum register.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param ctrl_hum Pointer to store the current value of ctrl_hum.
 * @return esp_err_t ESP_OK if successful, ESP_FAIL otherwise.
 */
esp_err_t bme280_read_ctrl_hum(BME280 *dev, uint8_t *ctrl_hum);

/**
 * @brief Set the config register (t_sb, filter, and spi3w_en settings).
 *
 * @param dev Pointer to the BME280 device structure.
 * @param t_sb Standby time setting (3 bits, t_sb[2:0]).
 * @param filter IIR filter coefficient (3 bits, filter[2:0]).
 * @param spi3w_en Enable/disable 3-wire SPI interface (1 bit).
 * @return esp_err_t ESP_OK if successful, ESP_FAIL otherwise.
 */
esp_err_t bme280_set_config(BME280 *dev, uint8_t t_sb, uint8_t filter, uint8_t spi3w_en);

/**
 * @brief Read the current value of the config register.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param config Pointer to store the current value of the config register.
 * @return esp_err_t ESP_OK if successful, ESP_FAIL otherwise.
 */
esp_err_t bme280_read_config(BME280 *dev, uint8_t *config);


/**
 * @brief Read the status register of the BME280.
 *
 * @param dev Pointer to the BME280 device structure.
 * @param status Pointer to store the raw value of the status register.
 * @return esp_err_t ESP_OK if successful, ESP_FAIL otherwise.
 */
esp_err_t bme280_read_status(BME280 *dev, uint8_t *status);

/**
 * @brief Interpret the status register values.
 *
 * @param status The raw value of the status register.
 * @param measuring Pointer to store the state of the measuring bit (1 if measuring, 0 otherwise).
 * @param im_update Pointer to store the state of the im_update bit (1 if copying NVM, 0 otherwise).
 */
void bme280_interpret_status(uint8_t status, uint8_t *measuring, uint8_t *im_update);

/**
 * @brief Perform a soft reset of the BME280 device.
 *
 * @param dev Pointer to the BME280 device structure.
 * @return esp_err_t ESP_OK if reset was successful, ESP_FAIL otherwise.
 */
esp_err_t bme280_reset(BME280 *dev);

#endif // BME280_H
