// Implementation of the periodic sensor sampling task.

#include "sensor/sensor_task.hpp"
#include "sensor/sensor_node.hpp"
#include "sensor/config.hpp"
#include "sensor/logic.hpp"

#include "esp_log.h"

namespace app::sensor {

namespace {
constexpr const char* kTag = "sensor_task";
} // namespace

SensorTask::SensorTask(I2cBus& i2cBus,
                       TemperatureSensor& temperatureSensor,
                       LightSensor& lightSensor,
                       SensorNode& sensorNode)
    : m_i2cBus(i2cBus),
      m_temperatureSensor(temperatureSensor),
      m_lightSensor(lightSensor),
      m_sensorNode(sensorNode),
      m_readingQueue(nullptr),
      m_taskHandle(nullptr) {
}

SensorTask::~SensorTask() {
    if (m_taskHandle != nullptr) {
        vTaskDelete(m_taskHandle);
    }
    if (m_readingQueue != nullptr) {
        vQueueDelete(m_readingQueue);
    }
}

esp_err_t SensorTask::start() {
    m_readingQueue = xQueueCreate(kSensorQueueLength, sizeof(SensorReading));
    if (m_readingQueue == nullptr) {
        ESP_LOGE(kTag, "Failed to create sensor reading queue");
        return ESP_ERR_NO_MEM;
    }

    BaseType_t result = xTaskCreatePinnedToCore(
        &SensorTask::taskFunction,
        kSensorTaskName,
        kSensorTaskStackSize,
        this,
        kSensorTaskPriority,
        &m_taskHandle,
        kSensorTaskCoreId
    );

    if (result != pdPASS) {
        ESP_LOGE(kTag, "Failed to create sensor task");
        return ESP_FAIL;
    }

    ESP_LOGI(kTag, "Sensor task successfully started on core %d (sampling period %lu ms)",
             kSensorTaskCoreId, kSensorSamplePeriodMs);
    return ESP_OK;
}

QueueHandle_t SensorTask::getReadingQueue() const {
    return m_readingQueue;
}

void SensorTask::taskFunction(void* arg) {
    auto* self = static_cast<SensorTask*>(arg);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t periodTicks = pdMS_TO_TICKS(kSensorSamplePeriodMs);

    ESP_LOGI(kTag, "Sensor sampling loop started");

    while (true) {
        vTaskDelayUntil(&lastWakeTime, periodTicks);

        float temperature = 0.0f;
        uint32_t lux = 0;
        bool tempSuccess = false;
        bool lightSuccess = false;

        if (self->m_i2cBus.lock(kI2cMutexTimeoutTicks)) {
            esp_err_t err = self->m_temperatureSensor.readTemperature(temperature);
            if (err == ESP_OK) {
                tempSuccess = true;
            } else {
                ESP_LOGE(kTag, "Error reading temperature: %s", esp_err_to_name(err));
            }

            err = self->m_lightSensor.readLux(lux);
            if (err == ESP_OK) {
                lightSuccess = true;
            } else {
                ESP_LOGE(kTag, "Error reading light lux: %s", esp_err_to_name(err));
            }

            self->m_i2cBus.unlock();
        } else {
            ESP_LOGE(kTag, "Failed to acquire I2C mutex for reading sensors");
        }

        if (tempSuccess && lightSuccess) {
            uint8_t lightIntensity = luxToLightIntensity(lux, kMaxLuxThreshold);
            SensorReading reading = {
                .temperature = temperature,
                .lightIntensity = lightIntensity,
                .valid = true,
            };

            ESP_LOGI(kTag, "Sampled: Temp=%.2f C, Lux=%lu -> Intensity=%u%%",
                     temperature, lux, lightIntensity);

            self->m_sensorNode.reportReading(temperature, lightIntensity);
            xQueueOverwrite(self->m_readingQueue, &reading);
        } else {
            ESP_LOGW(kTag, "Sensor read failed; skipping cloud report to prevent bad data");
        }

        UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(nullptr);
        ESP_LOGI(kTag, "Sensor task stack high-water mark: %u words", static_cast<unsigned int>(highWaterMark));
    }
}

} // namespace app::sensor
