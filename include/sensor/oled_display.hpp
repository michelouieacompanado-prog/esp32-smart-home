// SSD1306 OLED display driver for rendering sensor readings and network status.

#pragma once

#include <cstdint>
#include "esp_err.h"
#include "driver/i2c.h"

namespace app::sensor {

class OledDisplay {
public:
    OledDisplay();
    ~OledDisplay();

    esp_err_t init(i2c_port_t port);
    void update(float temperature,
                uint8_t lightIntensity,
                bool valid,
                bool wifiConnected,
                bool mqttConnected,
                const char* timeString);
    void clear();

private:
    esp_err_t sendCommand(uint8_t command);
    esp_err_t sendData(const uint8_t* data, size_t length);
    void drawChar(uint8_t x, uint8_t page, char c);
    void drawString(uint8_t x, uint8_t page, const char* str);

    i2c_port_t m_port;
    bool m_initialized;
    uint8_t m_buffer[128 * 8];
};

} // namespace app::sensor
