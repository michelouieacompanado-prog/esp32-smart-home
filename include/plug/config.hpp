// Configuration parameters and FreeRTOS settings for the plug device.

#pragma once

#include <cstdint>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

namespace app::plug {

// Hardware GPIO pin definitions
constexpr gpio_num_t kRelayGpio = GPIO_NUM_19;
constexpr gpio_num_t kButtonGpio = GPIO_NUM_0;
constexpr gpio_num_t kLedGpio = GPIO_NUM_2;

// Active signal levels
constexpr uint32_t kRelayActiveLevel = 1;
constexpr uint32_t kLedActiveLevel = 1;
constexpr uint32_t kButtonActiveLevel = 0;

// FreeRTOS Software Timer settings
constexpr uint32_t kButtonDebouncePeriodMs = 50;

// FreeRTOS Queue settings
constexpr UBaseType_t kPlugEventQueueLength = 10;

// FreeRTOS Task settings
inline constexpr const char* kPlugTaskName = "plug_task";
constexpr uint32_t kPlugTaskStackSize = 4096;
constexpr UBaseType_t kPlugTaskPriority = 5;
constexpr BaseType_t kPlugTaskCoreId = 1;

// Device & Node identifiers for ESP RainMaker
inline constexpr const char* kNodeName = "Smart Plug";
inline constexpr const char* kNodeType = "esp.node.plug";
inline constexpr const char* kDeviceName = "Plug";
inline constexpr const char* kDeviceType = "esp.device.plug";
inline constexpr const char* kPowerParamName = "power";

} // namespace app::plug
