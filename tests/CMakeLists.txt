cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} "./*.cc")

add_executable(
    MqttTests
    ${TEST_SOURCES}
)

target_include_directories(MqttTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}../src")
target_include_directories(MqttTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}./")

target_link_libraries(
    MqttTests
    cpp_mqtt_client
    GTest::gtest_main
    mosquitto
)

include(GoogleTest)

gtest_discover_tests(MqttTests)