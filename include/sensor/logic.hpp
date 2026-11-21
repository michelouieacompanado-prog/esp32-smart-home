// Pure conversion functions for sensor metrics, suitable for unit testing.

#pragma once

#include <cstdint>

namespace app::sensor {

inline constexpr uint8_t luxToLightIntensity(uint32_t lux, uint32_t maxLux = 2048) {
    if (maxLux == 0) {
        return 0;
    }
    if (lux >= maxLux) {
        return 100;
    }
    return static_cast<uint8_t>((static_cast<uint64_t>(lux) * 100) / maxLux);
}

} // namespace app::sensor
