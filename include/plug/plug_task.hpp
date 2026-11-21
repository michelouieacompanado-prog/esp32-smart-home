// FreeRTOS task managing plug state transitions, relay control, and reporting.

#pragma once

#include "plug/relay.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace app::plug {

class PlugNode;

enum class PlugEventType {
    kToggle,
    kSetState
};

struct PlugEvent {
    PlugEventType type;
    bool targetState;
};

class PlugTask {
public:
    PlugTask(Relay& relay, PlugNode& plugNode);
    ~PlugTask();

    esp_err_t start();
    QueueHandle_t getEventQueue() const;

private:
    static void taskFunction(void* arg);

    Relay& m_relay;
    PlugNode& m_plugNode;
    QueueHandle_t m_eventQueue;
    TaskHandle_t m_taskHandle;
};

} // namespace app::plug
