// Data model representing unified sensor readings.

#pragma once

#include <cstdint>

namespace app::sensor {

struct SensorReading {
    float temperature;
    uint8_t lightIntensity;
    bool valid;
};

} // namespace app::sensor
