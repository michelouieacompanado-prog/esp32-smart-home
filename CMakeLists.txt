# Root CMakeLists.txt for ESP-IDF Project
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32-smart-home)

# Provide protocomm proto-c include directory to network_provisioning
if(TARGET __idf_espressif__network_provisioning)
    target_include_directories(__idf_espressif__network_provisioning PRIVATE
        "$ENV{IDF_PATH}/components/protocomm/proto-c"
    )
endif()

# Generate assembly wrappers for embedded certificates needed by PlatformIO SCons
set(EMBED_SCRIPT "$ENV{IDF_PATH}/tools/cmake/scripts/data_file_embed_asm.cmake")
set(CERTS_TO_EMBED
    "${CMAKE_CURRENT_LIST_DIR}/managed_components/espressif__esp_insights/server_certs/https_server.crt"
    "${CMAKE_CURRENT_LIST_DIR}/managed_components/espressif__esp_rainmaker/server_certs/rmaker_mqtt_server.crt"
    "${CMAKE_CURRENT_LIST_DIR}/managed_components/espressif__esp_rainmaker/server_certs/rmaker_claim_service_server.crt"
    "${CMAKE_CURRENT_LIST_DIR}/managed_components/espressif__esp_rainmaker/server_certs/rmaker_ota_server.crt"
)

foreach(CERT_FILE ${CERTS_TO_EMBED})
    if(EXISTS "${CERT_FILE}")
        get_filename_component(CERT_NAME "${CERT_FILE}" NAME)
        set(ASM_OUT "${CMAKE_BINARY_DIR}/${CERT_NAME}.S")
        execute_process(
            COMMAND "${CMAKE_COMMAND}"
                -DDATA_FILE=${CERT_FILE}
                -DSOURCE_FILE=${ASM_OUT}
                -DFILE_TYPE=TEXT
                -P "${EMBED_SCRIPT}"
        )
    endif()
endforeach()
