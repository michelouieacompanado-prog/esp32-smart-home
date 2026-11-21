// Startup wiring for the environmental sensor application.

#include "common/network.hpp"
#include "sensor/config.hpp"
#include "sensor/i2c_bus.hpp"
#include "sensor/temperature_sensor.hpp"
#include "sensor/light_sensor.hpp"
#include "sensor/oled_display.hpp"
#include "sensor/sensor_node.hpp"
#include "sensor/sensor_task.hpp"
#include "sensor/display_task.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace {
constexpr const char* kTag = "sensor_main";

EventGroupHandle_t g_eventGroup = nullptr;
app::Network g_network;
app::sensor::I2cBus g_i2cBus;
app::sensor::TemperatureSensor g_temperatureSensor;
app::sensor::LightSensor g_lightSensor;
app::sensor::OledDisplay g_oledDisplay;
app::sensor::SensorNode g_sensorNode;
app::sensor::SensorTask g_sensorTask(g_i2cBus, g_temperatureSensor, g_lightSensor, g_sensorNode);
app::sensor::DisplayTask* g_displayTask = nullptr;
} // namespace

extern "C" void app_main() {
    ESP_LOGI(kTag, "Starting ESP32 Environmental Sensor firmware...");

    g_eventGroup = xEventGroupCreate();
    if (g_eventGroup == nullptr) {
        ESP_LOGE(kTag, "Failed to create network event group");
        return;
    }

    ESP_ERROR_CHECK(g_i2cBus.init());

    if (g_i2cBus.lock()) {
        ESP_ERROR_CHECK(g_oledDisplay.init(g_i2cBus.getPort()));
        ESP_ERROR_CHECK(g_temperatureSensor.init(g_i2cBus.getPort()));
        ESP_ERROR_CHECK(g_lightSensor.init(g_i2cBus.getPort()));
        g_i2cBus.unlock();
    }

    ESP_ERROR_CHECK(g_sensorNode.init(g_eventGroup, app::sensor::kMqttConnectedBit));
    ESP_ERROR_CHECK(g_sensorTask.start());

    static app::sensor::DisplayTask displayTask(
        g_i2cBus,
        g_oledDisplay,
        g_sensorTask.getReadingQueue(),
        g_eventGroup
    );
    g_displayTask = &displayTask;
    ESP_ERROR_CHECK(g_displayTask->start());

    ESP_ERROR_CHECK(g_network.init(g_eventGroup, app::sensor::kWifiConnectedBit));
    ESP_ERROR_CHECK(g_network.start());
    ESP_ERROR_CHECK(g_sensorNode.start());

    ESP_LOGI(kTag, "Environmental Sensor initialization complete");
}
