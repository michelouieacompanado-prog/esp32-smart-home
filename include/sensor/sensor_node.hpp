// RainMaker node implementation for environmental sensors.

#pragma once

#include "common/rainmaker_node.hpp"
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_params.h"

namespace app::sensor {

class SensorNode : public RainMakerNode {
public:
    SensorNode();
    ~SensorNode() override;

    esp_err_t reportReading(float temperature, uint8_t lightIntensity);

protected:
    void onDeviceInit(esp_rmaker_device_t* device) override;

private:
    esp_rmaker_param_t* m_temperatureParam;
    esp_rmaker_param_t* m_lightIntensityParam;
};

} // namespace app::sensor
