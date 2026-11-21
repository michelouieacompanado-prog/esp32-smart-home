// Network management for NVS, Wi-Fi initialization, and BLE provisioning.

#pragma once

#include "esp_err.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace app {

class Network {
public:
    Network();
    ~Network();

    esp_err_t init(EventGroupHandle_t eventGroup = nullptr, EventBits_t connectedBit = 0);
    esp_err_t start();
    bool isConnected() const;

private:
    static void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

    EventGroupHandle_t m_eventGroup;
    EventBits_t m_connectedBit;
    bool m_connected;
};

} // namespace app
