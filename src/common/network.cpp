// Implementation of network initialization, NVS management, and Wi-Fi/BLE provisioning.

#include "common/network.hpp"

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "app_wifi.h"

namespace app {

namespace {
constexpr const char* kTag = "network";
} // namespace

Network::Network()
    : m_eventGroup(nullptr),
      m_connectedBit(0),
      m_connected(false) {
}

Network::~Network() {
}

esp_err_t Network::init(EventGroupHandle_t eventGroup, EventBits_t connectedBit) {
    m_eventGroup = eventGroup;
    m_connectedBit = connectedBit;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(kTag, "NVS flash partition truncated/new version found, erasing...");
        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(kTag, "Failed to erase NVS flash: %s", esp_err_to_name(err));
            return err;
        }
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to initialize NVS: %s", esp_err_to_name(err));
        return err;
    }

    esp_event_handler_instance_t ipHandlerInstance;
    esp_event_handler_instance_t wifiHandlerInstance;
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &Network::wifiEventHandler, this, &ipHandlerInstance);
    esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
                                        &Network::wifiEventHandler, this, &wifiHandlerInstance);

    app_wifi_init();
    ESP_LOGI(kTag, "Network subsystem initialized");
    return ESP_OK;
}

esp_err_t Network::start() {
    ESP_LOGI(kTag, "Starting Wi-Fi / BLE provisioning");
    esp_err_t err = app_wifi_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to start provisioning: %s", esp_err_to_name(err));
    }
    return err;
}

bool Network::isConnected() const {
    return m_connected;
}

void Network::wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {
    auto* self = static_cast<Network*>(arg);
    if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        self->m_connected = true;
        ESP_LOGI(kTag, "Wi-Fi station obtained IP address");
        if (self->m_eventGroup != nullptr && self->m_connectedBit != 0) {
            xEventGroupSetBits(self->m_eventGroup, self->m_connectedBit);
        }
    } else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        self->m_connected = false;
        ESP_LOGW(kTag, "Wi-Fi station disconnected");
        if (self->m_eventGroup != nullptr && self->m_connectedBit != 0) {
            xEventGroupClearBits(self->m_eventGroup, self->m_connectedBit);
        }
    }
}

} // namespace app
