// Implementation of the plug RainMaker node and write callbacks.

#include "plug/plug_node.hpp"
#include "plug/config.hpp"
#include "plug/plug_task.hpp"

#include "esp_log.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"

namespace app::plug {

namespace {
constexpr const char* kTag = "plug_node";
} // namespace

PlugNode::PlugNode()
    : RainMakerNode({kNodeName, kNodeType, kDeviceName, kDeviceType}),
      m_powerParam(nullptr),
      m_eventQueue(nullptr) {
}

PlugNode::~PlugNode() {
}

void PlugNode::setEventQueue(QueueHandle_t eventQueue) {
    m_eventQueue = eventQueue;
}

esp_err_t PlugNode::reportState(bool state) {
    if (m_powerParam != nullptr) {
        ESP_LOGI(kTag, "Reporting power parameter to RainMaker: %s", state ? "true" : "false");
        return esp_rmaker_param_update_and_report(m_powerParam, esp_rmaker_bool(state));
    }
    return ESP_ERR_INVALID_STATE;
}

void PlugNode::onDeviceInit(esp_rmaker_device_t* device) {
    m_powerParam = esp_rmaker_power_param_create(kPowerParamName, false);
    esp_rmaker_device_add_param(device, m_powerParam);
    esp_rmaker_device_assign_primary_param(device, m_powerParam);
    esp_rmaker_device_add_cb(device, &PlugNode::writeCallback, nullptr);
    ESP_LOGI(kTag, "Plug RainMaker device and power parameter registered");
}

esp_err_t PlugNode::writeCallback(const esp_rmaker_device_t* device,
                                  const esp_rmaker_param_t* param,
                                  const esp_rmaker_param_val_t val,
                                  void* privateData,
                                  esp_rmaker_write_ctx_t* ctx) {
    auto* self = static_cast<PlugNode*>(privateData);
    if (self == nullptr || self->m_eventQueue == nullptr) {
        return ESP_FAIL;
    }

    ESP_LOGI(kTag, "Received cloud write for power: %s", val.val.b ? "ON" : "OFF");
    PlugEvent event = {
        .type = PlugEventType::kSetState,
        .targetState = val.val.b,
    };
    xQueueSend(self->m_eventQueue, &event, 0);
    return ESP_OK;
}

} // namespace app::plug
