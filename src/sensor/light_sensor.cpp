// Implementation of the TSL2561 light sensor interface.

#include "sensor/light_sensor.hpp"
#include "sensor/config.hpp"

#include <cstring>
#include "esp_log.h"

namespace app::sensor {

namespace {
constexpr const char* kTag = "light_sensor";
} // namespace

LightSensor::LightSensor()
    : m_initialized(false) {
    std::memset(&m_device, 0, sizeof(m_device));
}

LightSensor::~LightSensor() {
}

esp_err_t LightSensor::init(i2c_port_t port) {
    esp_err_t err = tsl2561_init_desc(&m_device, kTsl2561I2cAddress, port, kI2cSdaGpio, kI2cSclGpio);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to initialize TSL2561 descriptor: %s", esp_err_to_name(err));
        return err;
    }

    err = tsl2561_init(&m_device);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to initialize TSL2561 sensor: %s", esp_err_to_name(err));
        return err;
    }

    m_initialized = true;
    ESP_LOGI(kTag, "TSL2561 sensor initialized successfully at address 0x%02X", kTsl2561I2cAddress);
    return ESP_OK;
}

esp_err_t LightSensor::readLux(uint32_t& lux) {
    if (!m_initialized) {
        ESP_LOGE(kTag, "TSL2561 sensor not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = tsl2561_read_lux(&m_device, &lux);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to read lux from TSL2561: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGD(kTag, "Read lux: %lu", lux);
    return ESP_OK;
}

} // namespace app::sensor
