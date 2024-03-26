set(PICO_BOARD pico_w)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

SET(CPP_MQTT_STATIC OFF CACHE BOOL "")
SET(CPP_MQTT_SHARED ON CACHE BOOL "")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "include")

# initialize the Raspberry Pi Pico SDK
IF(${BUILD_TARGET} STREQUAL "PICO")
    pico_sdk_init()
ENDIF()

# Finding all of our source
file(GLOB_RECURSE SOURCES ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} "./*.cpp")
file(GLOB_RECURSE HEADERS ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

IF(CPP_MQTT_STATIC)
    add_library(cpp_mqtt_client STATIC ${SOURCES} ${HEADERS})
ELSEIF(CPP_MQTT_SHARED)
    add_library(cpp_mqtt_client SHARED ${SOURCES} ${HEADERS})
ENDIF()

target_include_directories(cpp_mqtt_client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")

IF(${BUILD_TARGET} STREQUAL "PICO")
    target_link_libraries(cpp_mqtt_client
        pico_stdlib
    )
ENDIF()