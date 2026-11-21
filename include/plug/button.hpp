// Push button driver with hardware interrupt and FreeRTOS software timer debounce.

#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

namespace app::plug {

class Button {
public:
    Button();
    ~Button();

    esp_err_t init(QueueHandle_t eventQueue);

private:
    static void IRAM_ATTR isrHandler(void* arg);
    static void debounceTimerCallback(TimerHandle_t timer);

    QueueHandle_t m_eventQueue;
    TimerHandle_t m_debounceTimer;
};

} // namespace app::plug
