set(PICO_BOARD pico_w)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS ON)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "include")

# initialize the Raspberry Pi Pico SDK
IF(${BUILD_TARGET} STREQUAL "PICO")
    pico_sdk_init()
ENDIF()

# Finding all of our source
file(GLOB_RECURSE SOURCES ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} "./*.cpp")
file(GLOB_RECURSE HEADERS ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

add_library(cpp_mqtt_client STATIC ${SOURCES} ${HEADERS})
target_include_directories(cpp_mqtt_client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

IF(${BUILD_TARGET} STREQUAL "PICO")
    target_link_libraries(cpp_mqtt_client
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