// Implementation of the sensor RainMaker node.

#include "sensor/sensor_node.hpp"
#include "sensor/config.hpp"

#include "esp_log.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"

namespace app::sensor {

namespace {
constexpr const char* kTag = "sensor_node";
} // namespace

SensorNode::SensorNode()
    : RainMakerNode({kNodeName, kNodeType, kDeviceName, kDeviceType}),
      m_temperatureParam(nullptr),
      m_lightIntensityParam(nullptr) {
}

SensorNode::~SensorNode() {
}

void SensorNode::onDeviceInit(esp_rmaker_device_t* device) {
    m_temperatureParam = esp_rmaker_temperature_param_create(kTemperatureParamName, 0.0f);
    esp_rmaker_device_add_param(device, m_temperatureParam);
    esp_rmaker_device_assign_primary_param(device, m_temperatureParam);

    m_lightIntensityParam = esp_rmaker_intensity_param_create(kLightIntensityParamName, 0);
    esp_rmaker_device_add_param(device, m_lightIntensityParam);

    ESP_LOGI(kTag, "Sensor RainMaker parameters registered (temperature & light-intensity)");
}

esp_err_t SensorNode::reportReading(float temperature, uint8_t lightIntensity) {
    if (!m_connected) {
        ESP_LOGD(kTag, "Skipping report: RainMaker not connected");
        return ESP_OK;
    }

    if (m_temperatureParam != nullptr) {
        esp_rmaker_param_update_and_report(m_temperatureParam, esp_rmaker_float(temperature));
    }
    if (m_lightIntensityParam != nullptr) {
        esp_rmaker_param_update_and_report(m_lightIntensityParam, esp_rmaker_int(lightIntensity));
    }

    ESP_LOGI(kTag, "Reported to cloud -> Temp: %.2f C, Light: %u %%", temperature, lightIntensity);
    return ESP_OK;
}

} // namespace app::sensor
