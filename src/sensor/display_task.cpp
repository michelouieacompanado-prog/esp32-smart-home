// Implementation of the display task and clock timer.

#include "sensor/display_task.hpp"
#include "sensor/config.hpp"

#include <ctime>
#include "esp_log.h"

namespace app::sensor {

namespace {
constexpr const char* kTag = "display_task";
} // namespace

DisplayTask::DisplayTask(I2cBus& i2cBus,
                         OledDisplay& display,
                         QueueHandle_t readingQueue,
                         EventGroupHandle_t eventGroup)
    : m_i2cBus(i2cBus),
      m_display(display),
      m_readingQueue(readingQueue),
      m_eventGroup(eventGroup),
      m_clockTimer(nullptr),
      m_taskHandle(nullptr),
      m_latestReading{0.0f, 0, false} {
}

DisplayTask::~DisplayTask() {
    if (m_clockTimer != nullptr) {
        xTimerDelete(m_clockTimer, portMAX_DELAY);
    }
    if (m_taskHandle != nullptr) {
        vTaskDelete(m_taskHandle);
    }
}

esp_err_t DisplayTask::start() {
    BaseType_t result = xTaskCreatePinnedToCore(
        &DisplayTask::taskFunction,
        kDisplayTaskName,
        kDisplayTaskStackSize,
        this,
        kDisplayTaskPriority,
        &m_taskHandle,
        kDisplayTaskCoreId
    );

    if (result != pdPASS) {
        ESP_LOGE(kTag, "Failed to create display task");
        return ESP_FAIL;
    }

    m_clockTimer = xTimerCreate(
        "clk_tmr",
        pdMS_TO_TICKS(kClockTimerPeriodMs),
        pdTRUE,
        this,
        &DisplayTask::clockTimerCallback
    );

    if (m_clockTimer == nullptr) {
        ESP_LOGE(kTag, "Failed to create clock timer");
        return ESP_ERR_NO_MEM;
    }

    if (xTimerStart(m_clockTimer, 0) != pdPASS) {
        ESP_LOGE(kTag, "Failed to start clock timer");
        return ESP_FAIL;
    }

    ESP_LOGI(kTag, "Display task started on core %d with 1s clock timer", kDisplayTaskCoreId);
    return ESP_OK;
}

void DisplayTask::clockTimerCallback(TimerHandle_t timer) {
    auto* self = static_cast<DisplayTask*>(pvTimerGetTimerID(timer));
    if (self->m_taskHandle != nullptr) {
        xTaskNotifyGive(self->m_taskHandle);
    }
}

void DisplayTask::taskFunction(void* arg) {
    auto* self = static_cast<DisplayTask*>(arg);
    ESP_LOGI(kTag, "Display task loop running");

    while (true) {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(kDisplayRefreshPeriodMs));

        // Check for latest sensor readings
        if (self->m_readingQueue != nullptr) {
            SensorReading incoming;
            if (xQueuePeek(self->m_readingQueue, &incoming, 0) == pdTRUE) {
                self->m_latestReading = incoming;
            }
        }

        // Check network event group status
        bool wifiConnected = false;
        bool mqttConnected = false;
        if (self->m_eventGroup != nullptr) {
            EventBits_t bits = xEventGroupGetBits(self->m_eventGroup);
            wifiConnected = ((bits & kWifiConnectedBit) != 0);
            mqttConnected = ((bits & kMqttConnectedBit) != 0);
        }

        // Format current timestamp
        std::time_t now = 0;
        std::time(&now);
        struct std::tm timeInfo = {};
        localtime_r(&now, &timeInfo);
        char timeString[16] = {0};
        std::strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeInfo);

        // Update OLED display with I2C bus mutex protection
        if (self->m_i2cBus.lock(kI2cMutexTimeoutTicks)) {
            self->m_display.update(
                self->m_latestReading.temperature,
                self->m_latestReading.lightIntensity,
                self->m_latestReading.valid,
                wifiConnected,
                mqttConnected,
                timeString
            );
            self->m_i2cBus.unlock();
        } else {
            ESP_LOGW(kTag, "Could not acquire I2C mutex for display refresh");
        }

        UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(nullptr);
        ESP_LOGI(kTag, "Display task stack high-water mark: %u words", static_cast<unsigned int>(highWaterMark));
    }
}

} // namespace app::sensor
