# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()

project(cpp_mqtt_client C CXX ASM)

set(PICO_BOARD pico_w)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# initialize the Raspberry Pi Pico SDK
IF(${MQTT_BUILD_TARGET} STREQUAL "PICO")
    pico_sdk_init()
ENDIF()

# Finding all of our source
file(GLOB_RECURSE SOURCES ABSOLUTE ${CMAKE_SOURCE_DIR} "./*.cpp")

# add_executable(cpp_mqtt_client ${SOURCES})
add_library(cpp_mqtt_client STATIC ${SOURCES})

IF(${MQTT_BUILD_TARGET} STREQUAL "PICO")
    # create map/bin/hex file etc.
    # pull in common dependencies
    target_link_libraries(cpp_mqtt_client
        pico_multicore
        pico_stdlib
    )

    IF(MQTT_PICO_POLL)
        target_link_libraries(cpp_mqtt_client
            pico_cyw43_arch_lwip_poll
        )

    ELSE()
        target_link_libraries(cpp_mqtt_client
            pico_cyw43_arch_none
        )
    ENDIF()
ENDIF()

target_include_directories(cpp_mqtt_client PUBLIC "${CMAKE_SOURCE_DIR}/include")

IF(${MQTT_BUILD_TARGET} STREQUAL "PICO")
    # create map/bin/hex file etc.
    pico_add_extra_outputs(cpp_mqtt_client)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(cpp_mqtt_client 1)
    pico_enable_stdio_uart(cpp_mqtt_client 0)
ENDIF()
