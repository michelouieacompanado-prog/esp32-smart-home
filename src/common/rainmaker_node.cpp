// Implementation of base ESP RainMaker node integration.

#include "common/rainmaker_node.hpp"

#include "esp_log.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_ota.h"
#include "esp_rmaker_schedule.h"
#include "esp_rmaker_scenes.h"
#include "esp_rmaker_utils.h"
#include "esp_rmaker_common_events.h"
#include "app_insights.h"

namespace app {

namespace {
constexpr const char* kTag = "rainmaker_node";
constexpr const char* kDefaultTimezone = "UTC";
constexpr uint32_t kSystemRebootSeconds = 2;
constexpr uint32_t kSystemResetSeconds = 2;
} // namespace

RainMakerNode::RainMakerNode(const NodeConfiguration& configuration)
    : m_configuration(configuration),
      m_nodeHandle(nullptr),
      m_deviceHandle(nullptr),
      m_eventGroup(nullptr),
      m_mqttBit(0),
      m_connected(false) {
}

RainMakerNode::~RainMakerNode() {
}

esp_err_t RainMakerNode::init(EventGroupHandle_t eventGroup, EventBits_t mqttBit) {
    m_eventGroup = eventGroup;
    m_mqttBit = mqttBit;

    esp_event_handler_instance_t rmakerInstance;
    esp_event_handler_instance_t otaInstance;
    esp_event_handler_instance_register(RMAKER_COMMON_EVENT, ESP_EVENT_ANY_ID,
                                        &RainMakerNode::rmakerEventHandler, this, &rmakerInstance);
    esp_event_handler_instance_register(RMAKER_OTA_EVENT, ESP_EVENT_ANY_ID,
                                        &RainMakerNode::rmakerEventHandler, this, &otaInstance);

    esp_rmaker_time_set_timezone(kDefaultTimezone);

    esp_rmaker_config_t rainmakerConfig = {
        .enable_time_sync = true,
    };

    m_nodeHandle = esp_rmaker_node_init(&rainmakerConfig, m_configuration.nodeName.c_str(),
                                        m_configuration.nodeType.c_str());
    if (m_nodeHandle == nullptr) {
        ESP_LOGE(kTag, "Failed to initialize RainMaker node");
        return ESP_FAIL;
    }

    m_deviceHandle = esp_rmaker_device_create(m_configuration.deviceName.c_str(),
                                              m_configuration.deviceType.c_str(),
                                              this);
    if (m_deviceHandle == nullptr) {
        ESP_LOGE(kTag, "Failed to create RainMaker device");
        return ESP_FAIL;
    }

    esp_rmaker_device_add_param(m_deviceHandle,
                                esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM,
                                                             m_configuration.deviceName.c_str()));

    onDeviceInit(m_deviceHandle);
    esp_rmaker_node_add_device(m_nodeHandle, m_deviceHandle);

    esp_rmaker_ota_config_t otaConfig = {};
    otaConfig.server_cert = ESP_RMAKER_OTA_DEFAULT_SERVER_CERT;
    esp_rmaker_ota_enable(&otaConfig, OTA_USING_PARAMS);

    esp_rmaker_system_serv_config_t systemConfig = {
        .flags = RMAKER_EVENT_REBOOT | RMAKER_EVENT_WIFI_RESET,
        .reboot_seconds = kSystemRebootSeconds,
        .reset_seconds = kSystemResetSeconds,
        .reset_reboot_seconds = kSystemRebootSeconds,
    };
    esp_rmaker_system_service_enable(&systemConfig);

    app_insights_enable();
    esp_rmaker_schedule_enable();
    esp_rmaker_scenes_enable();

    ESP_LOGI(kTag, "RainMaker node initialized successfully");
    return ESP_OK;
}

esp_err_t RainMakerNode::start() {
    ESP_LOGI(kTag, "Starting RainMaker service");
    return esp_rmaker_start();
}

bool RainMakerNode::isConnected() const {
    return m_connected;
}

esp_rmaker_node_t* RainMakerNode::getNodeHandle() const {
    return m_nodeHandle;
}

esp_rmaker_device_t* RainMakerNode::getDeviceHandle() const {
    return m_deviceHandle;
}

void RainMakerNode::onMqttConnected() {
}

void RainMakerNode::onMqttDisconnected() {
}

void RainMakerNode::rmakerEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {
    auto* self = static_cast<RainMakerNode*>(arg);
    if (eventBase == RMAKER_COMMON_EVENT) {
        switch (eventId) {
            case RMAKER_MQTT_EVENT_CONNECTED:
                self->m_connected = true;
                ESP_LOGI(kTag, "RainMaker MQTT client connected");
                if (self->m_eventGroup != nullptr && self->m_mqttBit != 0) {
                    xEventGroupSetBits(self->m_eventGroup, self->m_mqttBit);
                }
                self->onMqttConnected();
                break;
            case RMAKER_MQTT_EVENT_DISCONNECTED:
                self->m_connected = false;
                ESP_LOGW(kTag, "RainMaker MQTT client disconnected");
                if (self->m_eventGroup != nullptr && self->m_mqttBit != 0) {
                    xEventGroupClearBits(self->m_eventGroup, self->m_mqttBit);
                }
                self->onMqttDisconnected();
                break;
            default:
                break;
        }
    }
}

} // namespace app
