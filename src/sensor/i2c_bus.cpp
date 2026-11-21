// Implementation of thread-safe I2C bus driver using FreeRTOS mutex.

#include "sensor/i2c_bus.hpp"
#include "sensor/config.hpp"

#include "esp_log.h"
#include "i2cdev.h"

namespace app::sensor {

namespace {
constexpr const char* kTag = "i2c_bus";
} // namespace

I2cBus::I2cBus()
    : m_mutex(nullptr),
      m_port(kI2cPort),
      m_initialized(false) {
}

I2cBus::~I2cBus() {
    if (m_initialized) {
        i2c_driver_delete(m_port);
    }
    if (m_mutex != nullptr) {
        vSemaphoreDelete(m_mutex);
    }
}

esp_err_t I2cBus::init() {
    if (m_initialized) {
        return ESP_OK;
    }

    m_mutex = xSemaphoreCreateMutex();
    if (m_mutex == nullptr) {
        ESP_LOGE(kTag, "Failed to create I2C bus mutex");
        return ESP_ERR_NO_MEM;
    }

    i2c_config_t config = {};
    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = kI2cSdaGpio;
    config.scl_io_num = kI2cSclGpio;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = kI2cClockSpeedHz;

    esp_err_t err = i2c_param_config(m_port, &config);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to configure I2C parameters: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(m_port, config.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to install I2C driver: %s", esp_err_to_name(err));
        return err;
    }

    err = i2cdev_init();
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to initialize i2cdev library: %s", esp_err_to_name(err));
        return err;
    }

    m_initialized = true;
    ESP_LOGI(kTag, "I2C bus initialized successfully on SDA=%d, SCL=%d", kI2cSdaGpio, kI2cSclGpio);
    return ESP_OK;
}

bool I2cBus::lock(TickType_t timeoutTicks) {
    if (m_mutex == nullptr) {
        return false;
    }
    return (xSemaphoreTake(m_mutex, timeoutTicks) == pdTRUE);
}

void I2cBus::unlock() {
    if (m_mutex != nullptr) {
        xSemaphoreGive(m_mutex);
    }
}

SemaphoreHandle_t I2cBus::getMutex() const {
    return m_mutex;
}

i2c_port_t I2cBus::getPort() const {
    return m_port;
}

} // namespace app::sensor
