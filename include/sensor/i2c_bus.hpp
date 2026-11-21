// Thread-safe I2C bus controller with FreeRTOS mutex synchronization.

#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "sensor/config.hpp"

namespace app::sensor {

class I2cBus {
public:
    I2cBus();
    ~I2cBus();

    esp_err_t init();
    bool lock(TickType_t timeoutTicks = kI2cMutexTimeoutTicks);
    void unlock();

    SemaphoreHandle_t getMutex() const;
    i2c_port_t getPort() const;

private:
    SemaphoreHandle_t m_mutex;
    i2c_port_t m_port;
    bool m_initialized;
};

} // namespace app::sensor
