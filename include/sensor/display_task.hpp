// FreeRTOS task managing SSD1306 OLED rendering and status updates.

#pragma once

#include "sensor/i2c_bus.hpp"
#include "sensor/oled_display.hpp"
#include "sensor/reading.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

namespace app::sensor {

class DisplayTask {
public:
    DisplayTask(I2cBus& i2cBus,
                OledDisplay& display,
                QueueHandle_t readingQueue,
                EventGroupHandle_t eventGroup);
    ~DisplayTask();

    esp_err_t start();

private:
    static void taskFunction(void* arg);
    static void clockTimerCallback(TimerHandle_t timer);

    I2cBus& m_i2cBus;
    OledDisplay& m_display;
    QueueHandle_t m_readingQueue;
    EventGroupHandle_t m_eventGroup;
    TimerHandle_t m_clockTimer;
    TaskHandle_t m_taskHandle;
    SensorReading m_latestReading;
};

} // namespace app::sensor
