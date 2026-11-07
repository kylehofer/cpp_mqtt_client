cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} "./*.cc")

# The end-to-end tests talk to a real broker through libmosquitto. They are
# built only when its headers and library are found (e.g. libmosquitto-dev),
# so the unit tests still build without them.
find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h)
find_library(MOSQUITTO_LIBRARY mosquitto)

IF(MOSQUITTO_INCLUDE_DIR AND MOSQUITTO_LIBRARY)
    SET(CPP_MQTT_END_TO_END_TESTS ON)
    message(STATUS "libmosquitto found: building end-to-end tests")
ELSE()
    SET(CPP_MQTT_END_TO_END_TESTS OFF)
    message(STATUS "libmosquitto not found: skipping end-to-end tests (install libmosquitto-dev to enable)")
    list(FILTER TEST_SOURCES EXCLUDE REGEX ".*/(EndToEndTrial|utils/MosquittoTestClient|utils/MqttTestHandler)\\.cc$")
ENDIF()

add_executable(
    MqttTests
    ${TEST_SOURCES}
)

target_include_directories(MqttTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../src")
target_include_directories(MqttTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")

target_link_libraries(
    MqttTests
    cpp_mqtt_client
    GTest::gtest_main
)

IF(CPP_MQTT_END_TO_END_TESTS)
    target_include_directories(MqttTests PRIVATE ${MOSQUITTO_INCLUDE_DIR})
    target_link_libraries(MqttTests ${MOSQUITTO_LIBRARY})
ENDIF()

include(GoogleTest)

gtest_discover_tests(MqttTests)