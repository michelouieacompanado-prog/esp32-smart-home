// RainMaker node implementation for the smart plug device.

#pragma once

#include "common/rainmaker_node.hpp"
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_params.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace app::plug {

class PlugNode : public RainMakerNode {
public:
    PlugNode();
    ~PlugNode() override;

    void setEventQueue(QueueHandle_t eventQueue);
    esp_err_t reportState(bool state);

protected:
    void onDeviceInit(esp_rmaker_device_t* device) override;

private:
    static esp_err_t writeCallback(const esp_rmaker_device_t* device,
                                   const esp_rmaker_param_t* param,
                                   const esp_rmaker_param_val_t val,
                                   void* privateData,
                                   esp_rmaker_write_ctx_t* ctx);

    esp_rmaker_param_t* m_powerParam;
    QueueHandle_t m_eventQueue;
};

} // namespace app::plug
