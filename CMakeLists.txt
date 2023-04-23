# cmake_policy(SET CMP0057 NEW)
cmake_minimum_required(VERSION 3.17)

# # build options
SET(MQTT_BUILD_TARGET LINUX CACHE BOOL "")
SET(MQTT_BUILD_SHARED TRUE CACHE BOOL "")
SET(MQTT_BUILD_STATIC FALSE CACHE BOOL "")
SET(MQTT_ENABLE_TESTING TRUE CACHE BOOL "")
SET(MQTT_PICO_POLL TRUE CACHE BOOL "")

IF(${MQTT_BUILD_TARGET} STREQUAL "PICO")
    SET(CMAKE_SYSTEM_NAME Linux)
    SET(CMAKE_SYSTEM_PROCESSOR arm)
    SET(CMAKE_SYSTEM_VERSION 1)
    set(CMAKE_CROSSCOMPILING TRUE)

    set(CMAKE_FIND_ROOT_PATH /usr/bin/arm-none-eabi)

    # Cross compiler
    SET(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)
    SET(CMAKE_CXX_COMPILER /usr/bin/arm-none-eabi-g++)
    set(CMAKE_LIBRARY_ARCHITECTURE /usr/bin/arm-none-eabi)

    # Search for programs in the build host directories
    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

    # Libraries and headers in the target directories
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    set(THREADS_PTHREAD_ARG "0" CACHE STRING "Result from TRY_RUN" FORCE)

    include(pico_sdk_import.cmake)
ENDIF()

project(cpp_mqtt_client_tests C CXX ASM)

IF(MQTT_ENABLE_TESTING AND NOT ${MQTT_BUILD_TARGET} STREQUAL "PICO")
    enable_testing()
    add_subdirectory(./tests)
ELSEIF()
    add_subdirectory(./src)
ENDIF()
