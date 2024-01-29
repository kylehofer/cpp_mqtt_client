# cmake_policy(SET CMP0057 NEW)
cmake_minimum_required(VERSION 3.17)

# # build options
SET(BUILD_TARGET PICO CACHE BOOL "")
SET(MQTT_ENABLE_TESTING FALSE CACHE BOOL "")

IF(${BUILD_TARGET} STREQUAL "PICO")
    add_definitions(-DPICO)
    include(pico_sdk_import.cmake)
ENDIF()

project(cpp_mqtt C CXX ASM)

add_subdirectory(./src)

IF(MQTT_ENABLE_TESTING AND NOT(${BUILD_TARGET} STREQUAL "PICO"))
    enable_testing()
    add_subdirectory(./tests)
ENDIF()
