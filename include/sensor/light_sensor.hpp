// TSL2561 ambient light sensor driver wrapper.

#pragma once

#include "esp_err.h"
#include "tsl2561.h"

namespace app::sensor {

class LightSensor {
public:
    LightSensor();
    ~LightSensor();

    esp_err_t init(i2c_port_t port);
    esp_err_t readLux(uint32_t& lux);

private:
    tsl2561_t m_device;
    bool m_initialized;
};

} // namespace app::sensor
