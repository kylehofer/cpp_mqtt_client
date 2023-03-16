set(PICO_BOARD pico_w)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Finding all of our source
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "./*.cpp")

add_library(cpp_mqtt_client STATIC ${SOURCES})

# pull in common dependencies
target_link_libraries(cpp_mqtt_client
    pico_multicore
    pico_stdlib
    pico_cyw43_arch_lwip_poll

    # pico_cyw43_arch_none
)

target_include_directories(cpp_mqtt_client PUBLIC "${CMAKE_SOURCE_DIR}/include")

# create map/bin/hex file etc.
pico_add_extra_outputs(cpp_mqtt_client)

# enable usb output, disable uart output
pico_enable_stdio_usb(cpp_mqtt_client 1)
pico_enable_stdio_uart(cpp_mqtt_client 0)