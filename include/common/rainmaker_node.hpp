// Base class for ESP RainMaker nodes providing OTA, scheduling, scenes, and system services.

#pragma once

#include <string>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_rmaker_core.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace app {

struct NodeConfiguration {
    std::string nodeName;
    std::string nodeType;
    std::string deviceName;
    std::string deviceType;
};

class RainMakerNode {
public:
    explicit RainMakerNode(const NodeConfiguration& configuration);
    virtual ~RainMakerNode();

    virtual esp_err_t init(EventGroupHandle_t eventGroup = nullptr, EventBits_t mqttBit = 0);
    virtual esp_err_t start();

    bool isConnected() const;
    esp_rmaker_node_t* getNodeHandle() const;
    esp_rmaker_device_t* getDeviceHandle() const;

protected:
    virtual void onDeviceInit(esp_rmaker_device_t* device) = 0;
    virtual void onMqttConnected();
    virtual void onMqttDisconnected();

    NodeConfiguration m_configuration;
    esp_rmaker_node_t* m_nodeHandle;
    esp_rmaker_device_t* m_deviceHandle;
    EventGroupHandle_t m_eventGroup;
    EventBits_t m_mqttBit;
    bool m_connected;

private:
    static void rmakerEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
};

} // namespace app
