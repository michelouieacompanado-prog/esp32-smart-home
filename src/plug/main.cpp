// Startup wiring for the smart plug application.

#include "common/network.hpp"
#include "plug/relay.hpp"
#include "plug/button.hpp"
#include "plug/plug_node.hpp"
#include "plug/plug_task.hpp"

#include "esp_log.h"

namespace {
constexpr const char* kTag = "plug_main";

app::Network g_network;
app::plug::Relay g_relay;
app::plug::PlugNode g_plugNode;
app::plug::PlugTask g_plugTask(g_relay, g_plugNode);
app::plug::Button g_button;
} // namespace

extern "C" void app_main() {
    ESP_LOGI(kTag, "Starting ESP32 Smart Plug firmware...");

    ESP_ERROR_CHECK(g_relay.init());
    ESP_ERROR_CHECK(g_plugNode.init());
    ESP_ERROR_CHECK(g_plugTask.start());
    ESP_ERROR_CHECK(g_button.init(g_plugTask.getEventQueue()));
    ESP_ERROR_CHECK(g_network.init());
    ESP_ERROR_CHECK(g_network.start());
    ESP_ERROR_CHECK(g_plugNode.start());

    ESP_LOGI(kTag, "Smart Plug initialization complete");
}
