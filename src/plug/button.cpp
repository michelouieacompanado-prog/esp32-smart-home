// Implementation of the push button with interrupt and timer debounce.

#include "plug/button.hpp"
#include "plug/config.hpp"
#include "plug/plug_task.hpp"

#include "driver/gpio.h"
#include "esp_log.h"

namespace app::plug {

namespace {
constexpr const char* kTag = "button";
} // namespace

Button::Button()
    : m_eventQueue(nullptr),
      m_debounceTimer(nullptr) {
}

Button::~Button() {
    if (m_debounceTimer != nullptr) {
        xTimerDelete(m_debounceTimer, portMAX_DELAY);
    }
}

esp_err_t Button::init(QueueHandle_t eventQueue) {
    m_eventQueue = eventQueue;

    m_debounceTimer = xTimerCreate("btn_deb",
                                   pdMS_TO_TICKS(kButtonDebouncePeriodMs),
                                   pdFALSE,
                                   this,
                                   &Button::debounceTimerCallback);
    if (m_debounceTimer == nullptr) {
        ESP_LOGE(kTag, "Failed to create debounce timer");
        return ESP_ERR_NO_MEM;
    }

    gpio_config_t buttonConfig = {
        .pin_bit_mask = (1ULL << kButtonGpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    esp_err_t err = gpio_config(&buttonConfig);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to configure button GPIO %d: %s", kButtonGpio, esp_err_to_name(err));
        return err;
    }

    gpio_install_isr_service(0);
    err = gpio_isr_handler_add(kButtonGpio, &Button::isrHandler, this);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to register button ISR handler: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(kTag, "Button initialized on GPIO %d with %lu ms debounce",
             kButtonGpio, kButtonDebouncePeriodMs);
    return ESP_OK;
}

void IRAM_ATTR Button::isrHandler(void* arg) {
    auto* self = static_cast<Button*>(arg);
    BaseType_t highPriorityTaskWoken = pdFALSE;
    xTimerResetFromISR(self->m_debounceTimer, &highPriorityTaskWoken);
    portYIELD_FROM_ISR(highPriorityTaskWoken);
}

void Button::debounceTimerCallback(TimerHandle_t timer) {
    auto* self = static_cast<Button*>(pvTimerGetTimerID(timer));
    int level = gpio_get_level(kButtonGpio);
    if (level == static_cast<int>(kButtonActiveLevel)) {
        ESP_LOGI(kTag, "Button press debounced; queuing toggle event");
        PlugEvent event = {
            .type = PlugEventType::kToggle,
            .targetState = false,
        };
        if (self->m_eventQueue != nullptr) {
            xQueueSend(self->m_eventQueue, &event, 0);
        }
    }
}

} // namespace app::plug
