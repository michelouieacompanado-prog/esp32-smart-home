# CMakeLists.txt for src component - selects sources per device

set(TARGET_DEVICE "plug")
if(DEFINED DEVICE)
    set(TARGET_DEVICE "${DEVICE}")
elseif(DEFINED ENV{PIOENV})
    set(TARGET_DEVICE "$ENV{PIOENV}")
elseif(DEFINED ENV{DEVICE})
    set(TARGET_DEVICE "$ENV{DEVICE}")
endif()

message(STATUS "Selected ESP32 Smart Home device target: ${TARGET_DEVICE}")

# Common source files and include directories shared across both targets
set(COMPONENT_SRCS
    "common/network.cpp"
    "common/rainmaker_node.cpp"
)

set(COMPONENT_ADD_INCLUDEDIRS
    "../include/common"
)

set(COMPONENT_REQUIRES
    nvs_flash
    esp_wifi
    esp_event
    driver
    freertos
    esp_timer
    esp_rainmaker
    rmaker_app_network
    rmaker_app_insights
)

if(TARGET_DEVICE STREQUAL "plug")
    list(APPEND COMPONENT_SRCS
        "plug/main.cpp"
        "plug/relay.cpp"
        "plug/button.cpp"
        "plug/plug_task.cpp"
        "plug/plug_node.cpp"
    )
    list(APPEND COMPONENT_ADD_INCLUDEDIRS
        "../include/plug"
    )
elseif(TARGET_DEVICE STREQUAL "sensor")
    list(APPEND COMPONENT_SRCS
        "sensor/main.cpp"
        "sensor/i2c_bus.cpp"
        "sensor/temperature_sensor.cpp"
        "sensor/light_sensor.cpp"
        "sensor/oled_display.cpp"
        "sensor/sensor_task.cpp"
        "sensor/display_task.cpp"
        "sensor/sensor_node.cpp"
    )
    list(APPEND COMPONENT_ADD_INCLUDEDIRS
        "../include/sensor"
    )
    list(APPEND COMPONENT_REQUIRES
        bmp280
        tsl2561
        i2cdev
    )
else()
    message(FATAL_ERROR "Invalid device target '${TARGET_DEVICE}'. Expected 'plug' or 'sensor'.")
endif()

idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    REQUIRES ${COMPONENT_REQUIRES}
)
