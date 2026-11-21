// Implementation of the power relay and status LED driver.

#include "plug/relay.hpp"
#include "plug/config.hpp"

#include "driver/gpio.h"
#include "esp_log.h"

namespace app::plug {

namespace {
constexpr const char* kTag = "relay";
} // namespace

Relay::Relay()
    : m_state(false) {
}

Relay::~Relay() {
}

esp_err_t Relay::init() {
    gpio_config_t relayConfig = {
        .pin_bit_mask = (1ULL << kRelayGpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&relayConfig);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to configure relay GPIO %d: %s", kRelayGpio, esp_err_to_name(err));
        return err;
    }

    gpio_config_t ledConfig = {
        .pin_bit_mask = (1ULL << kLedGpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    err = gpio_config(&ledConfig);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "Failed to configure LED GPIO %d: %s", kLedGpio, esp_err_to_name(err));
        return err;
    }

    setState(false);
    ESP_LOGI(kTag, "Relay driver initialized on GPIO %d (LED on GPIO %d)", kRelayGpio, kLedGpio);
    return ESP_OK;
}

void Relay::setState(bool state) {
    m_state = state;
    gpio_set_level(kRelayGpio, state ? kRelayActiveLevel : !kRelayActiveLevel);
    gpio_set_level(kLedGpio, state ? kLedActiveLevel : !kLedActiveLevel);
    ESP_LOGI(kTag, "Relay state updated to: %s", state ? "ON" : "OFF");
}

bool Relay::getState() const {
    return m_state;
}

} // namespace app::plug
