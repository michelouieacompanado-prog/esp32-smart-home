// BMP280 temperature sensor driver wrapper.

#pragma once

#include "bmp280.h"
#include "esp_err.h"

namespace app::sensor {

class TemperatureSensor {
public:
    TemperatureSensor();
    ~TemperatureSensor();

    esp_err_t init(i2c_port_t port);
    esp_err_t readTemperature(float& temperature);

private:
    bmp280_t m_device;
    bmp280_params_t m_params;
    bool m_initialized;
};

} // namespace app::sensor
