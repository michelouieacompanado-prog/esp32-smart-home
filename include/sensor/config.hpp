// Configuration constants and FreeRTOS settings for the sensor device.

#pragma once

#include <cstdint>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace app::sensor {

// Hardware I2C pin and bus configurations
constexpr i2c_port_t kI2cPort = I2C_NUM_0;
constexpr gpio_num_t kI2cSdaGpio = GPIO_NUM_21;
constexpr gpio_num_t kI2cSclGpio = GPIO_NUM_22;
constexpr uint32_t kI2cClockSpeedHz = 100000;

// Device I2C slave addresses (7-bit)
constexpr uint8_t kBmp280I2cAddress = 0x76;
constexpr uint8_t kTsl2561I2cAddress = 0x39;
constexpr uint8_t kSsd1306I2cAddress = 0x3C;

// SSD1306 Display geometry
constexpr uint8_t kDisplayWidth = 128;
constexpr uint8_t kDisplayHeight = 64;

// FreeRTOS Task settings
inline constexpr const char* kSensorTaskName = "sensor_task";
constexpr uint32_t kSensorTaskStackSize = 4096;
constexpr UBaseType_t kSensorTaskPriority = 5;
constexpr BaseType_t kSensorTaskCoreId = 1;
constexpr uint32_t kSensorSamplePeriodMs = 10000;

inline constexpr const char* kDisplayTaskName = "display_task";
constexpr uint32_t kDisplayTaskStackSize = 4096;
constexpr UBaseType_t kDisplayTaskPriority = 4;
constexpr BaseType_t kDisplayTaskCoreId = 1;
constexpr uint32_t kDisplayRefreshPeriodMs = 1000;

// FreeRTOS Queue settings
constexpr UBaseType_t kSensorQueueLength = 1;

// FreeRTOS Software Timer settings
constexpr uint32_t kClockTimerPeriodMs = 1000;

// FreeRTOS Event Group bits for Network & Cloud status
constexpr EventBits_t kWifiConnectedBit = (1 << 0);
constexpr EventBits_t kMqttConnectedBit = (1 << 1);

// Light intensity calculation parameters
constexpr uint32_t kMaxLuxThreshold = 2048;
constexpr uint8_t kMaxLightIntensityPercent = 100;

// FreeRTOS Mutex timeout
constexpr TickType_t kI2cMutexTimeoutTicks = pdMS_TO_TICKS(1000);

// Device & Node identifiers for ESP RainMaker
inline constexpr const char* kNodeName = "Environmental Sensor";
inline constexpr const char* kNodeType = "esp.node.sensor";
inline constexpr const char* kDeviceName = "Sensor";
inline constexpr const char* kDeviceType = "esp.device.sensor";
inline constexpr const char* kTemperatureParamName = "temperature";
inline constexpr const char* kLightIntensityParamName = "light-intensity";

} // namespace app::sensor
