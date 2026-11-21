// FreeRTOS periodic sensor acquisition task.

#pragma once

#include "sensor/i2c_bus.hpp"
#include "sensor/temperature_sensor.hpp"
#include "sensor/light_sensor.hpp"
#include "sensor/reading.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace app::sensor {

class SensorNode;

class SensorTask {
public:
    SensorTask(I2cBus& i2cBus,
               TemperatureSensor& temperatureSensor,
               LightSensor& lightSensor,
               SensorNode& sensorNode);
    ~SensorTask();

    esp_err_t start();
    QueueHandle_t getReadingQueue() const;

private:
    static void taskFunction(void* arg);

    I2cBus& m_i2cBus;
    TemperatureSensor& m_temperatureSensor;
    LightSensor& m_lightSensor;
    SensorNode& m_sensorNode;
    QueueHandle_t m_readingQueue;
    TaskHandle_t m_taskHandle;
};

} // namespace app::sensor
