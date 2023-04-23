cmake_minimum_required(VERSION 3.14)

project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE SOURCES ABSOLUTE ${CMAKE_SOURCE_DIR} "../src/*.cpp")

file(GLOB_RECURSE TEST_SOURCES ABSOLUTE ${CMAKE_SOURCE_DIR} "./*.cc")
add_executable(
    MqttTests
    ${TEST_SOURCES}
    ${SOURCES}
)

target_include_directories(MqttTests PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(
    MqttTests
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(MqttTests)