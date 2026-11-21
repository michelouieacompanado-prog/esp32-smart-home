// Implementation of the plug FreeRTOS task.

#include "plug/plug_task.hpp"
#include "plug/config.hpp"
#include "plug/plug_node.hpp"

#include "esp_log.h"

namespace app::plug {

namespace {
constexpr const char* kTag = "plug_task";
} // namespace

PlugTask::PlugTask(Relay& relay, PlugNode& plugNode)
    : m_relay(relay),
      m_plugNode(plugNode),
      m_eventQueue(nullptr),
      m_taskHandle(nullptr) {
}

PlugTask::~PlugTask() {
    if (m_taskHandle != nullptr) {
        vTaskDelete(m_taskHandle);
    }
    if (m_eventQueue != nullptr) {
        vQueueDelete(m_eventQueue);
    }
}

esp_err_t PlugTask::start() {
    m_eventQueue = xQueueCreate(kPlugEventQueueLength, sizeof(PlugEvent));
    if (m_eventQueue == nullptr) {
        ESP_LOGE(kTag, "Failed to create plug event queue");
        return ESP_ERR_NO_MEM;
    }

    m_plugNode.setEventQueue(m_eventQueue);

    BaseType_t result = xTaskCreatePinnedToCore(
        &PlugTask::taskFunction,
        kPlugTaskName,
        kPlugTaskStackSize,
        this,
        kPlugTaskPriority,
        &m_taskHandle,
        kPlugTaskCoreId
    );

    if (result != pdPASS) {
        ESP_LOGE(kTag, "Failed to create plug task");
        return ESP_FAIL;
    }

    ESP_LOGI(kTag, "Plug task successfully started on core %d", kPlugTaskCoreId);
    return ESP_OK;
}

QueueHandle_t PlugTask::getEventQueue() const {
    return m_eventQueue;
}

void PlugTask::taskFunction(void* arg) {
    auto* self = static_cast<PlugTask*>(arg);
    PlugEvent event = {};

    ESP_LOGI(kTag, "Plug task running");

    while (true) {
        if (xQueueReceive(self->m_eventQueue, &event, portMAX_DELAY) == pdTRUE) {
            bool currentState = self->m_relay.getState();
            bool nextState = currentState;

            if (event.type == PlugEventType::kToggle) {
                nextState = !currentState;
                ESP_LOGI(kTag, "Processing toggle event -> %s", nextState ? "ON" : "OFF");
            } else if (event.type == PlugEventType::kSetState) {
                nextState = event.targetState;
                ESP_LOGI(kTag, "Processing set state event -> %s", nextState ? "ON" : "OFF");
            }

            self->m_relay.setState(nextState);
            self->m_plugNode.reportState(nextState);

            UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(nullptr);
            ESP_LOGI(kTag, "Stack high-water mark: %u words", static_cast<unsigned int>(highWaterMark));
        }
    }
}

} // namespace app::plug
