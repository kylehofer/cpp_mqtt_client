cmake_minimum_required(VERSION 3.17)

# # build options
SET(BUILD_TARGET PICO CACHE BOOL "")
SET(CPP_MQTT_TESTS FALSE CACHE BOOL "")

include(cmake/CPM.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

IF(NOT ${BUILD_TARGET} STREQUAL "PICO")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

IF(${BUILD_TARGET} STREQUAL "PICO")
    # set(PICO_SDK_FETCH_FROM_GIT ON)
    add_definitions(-DPICO)
    include(pico_sdk_import.cmake)
ENDIF()

CPMAddPackage(
        elapsed_time
        GIT_REPOSITORY git@github.com:kylehofer/elapsed_time.git
        GIT_TAG main
        OPTIONS
        "ELAPSED_TIME_STATIC ON"
    )

# 

project(cpp_mqtt C CXX ASM)

add_subdirectory(./src)

IF(CPP_MQTT_TESTS AND NOT(${BUILD_TARGET} STREQUAL "PICO"))
    enable_testing()
    add_subdirectory(./tests)
ENDIF()
