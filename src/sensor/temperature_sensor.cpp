// Implementation of the BMP280 temperature sensor interface.

#include "sensor/temperature_sensor.hpp"
#include "sensor/config.hpp"

#include <cstring>
#include "esp_log.h"

namespace app::sensor {

namespace {
constexpr const char* kTag = "temperature_sensor";
} // namespace

TemperatureSensor::TemperatureSensor()
    : m_initialized(false) {
    std::memset(&m_device, 0, sizeof(m_device));
    std::memset(&m_params, 0, sizeof(m_params));
}

TemperatureSensor::~TemperatureSensor() {
}

esp_err_t TemperatureSensor::init(i2c_port_t port) {
    esp_err_t err = bmp280_init_default_params(&m_params);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to load default BMP280 parameters: %s", esp_err_to_name(err));
        return err;
    }

    err = bmp280_init_desc(&m_device, kBmp280I2cAddress, port, kI2cSdaGpio, kI2cSclGpio);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to initialize BMP280 descriptor: %s", esp_err_to_name(err));
        return err;
    }

    err = bmp280_init(&m_device, &m_params);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to initialize BMP280 sensor: %s", esp_err_to_name(err));
        return err;
    }

    m_initialized = true;
    ESP_LOGI(kTag, "BMP280 sensor initialized successfully at address 0x%02X", kBmp280I2cAddress);
    return ESP_OK;
}

esp_err_t TemperatureSensor::readTemperature(float& temperature) {
    if (!m_initialized) {
        ESP_LOGE(kTag, "BMP280 sensor not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    float pressure = 0.0f;
    float humidity = 0.0f;
    esp_err_t err = bmp280_read_float(&m_device, &temperature, &pressure, &humidity);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to read temperature from BMP280: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGD(kTag, "Read temperature: %.2f degC", temperature);
    return ESP_OK;
}

} // namespace app::sensor
