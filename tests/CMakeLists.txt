cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

FetchContent_MakeAvailable(googletest)

# Grabbing mosquitto
FetchContent_Declare(mosquitto_external
    GIT_REPOSITORY https://github.com/eclipse/mosquitto.git
    GIT_TAG b0277869d9806f6fab8e1bc11c4a4987c9a79ded # v2.0.15
)

FetchContent_MakeAvailable(mosquitto_external)

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
    mosquitto_external
)

include(GoogleTest)

gtest_discover_tests(MqttTests)