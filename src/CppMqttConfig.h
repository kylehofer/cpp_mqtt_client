/*
 * File: CppMqttConfig.h
 * Project: cpp_mqtt_client
 * Author: Kyle Hofer
 *
 * MIT License
 *
 * Copyright (c) 2023 Kyle Hofer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Build configuration for cpp_mqtt_client.
 *
 * Memory mode
 *   Define CPP_MQTT_STATIC_MEMORY (or the older STATIC_MEMORY) to build without
 *   any dynamic allocation. Every buffer and container then has a fixed size set
 *   by the limits below. The define must be the same for the library and for
 *   every file that includes its headers, because it changes object layouts:
 *     - CMake: set CPP_MQTT_DYNAMIC_MEMORY=OFF; the define is exported to targets
 *       that link cpp_mqtt_client.
 *     - PlatformIO: add -DCPP_MQTT_STATIC_MEMORY to build_flags in platformio.ini,
 *       which applies to the application and its libraries.
 *   A mismatch fails at link time with an undefined reference to
 *   cpp_mqtt_memory_mode_mismatch_*, instead of corrupting memory at run time.
 *
 * Limits
 *   Each limit can be overridden with -D<NAME>=<value>. Those marked "static"
 *   only apply with CPP_MQTT_STATIC_MEMORY.
 */

#ifndef SRC_CPPMQTTCONFIG
#define SRC_CPPMQTTCONFIG

#include <stddef.h>

#if defined(STATIC_MEMORY) && !defined(CPP_MQTT_STATIC_MEMORY)
#define CPP_MQTT_STATIC_MEMORY
#endif
#if defined(CPP_MQTT_STATIC_MEMORY) && !defined(STATIC_MEMORY)
// The library's sources test STATIC_MEMORY
#define STATIC_MEMORY
#endif

// static: bytes stored for each UTF-8 string or binary field (topic, client id,
// property strings). Longer received values are truncated and flagged.
#ifndef CPP_MQTT_MAX_STRING_LENGTH
#ifdef MAX_BUFFER_SIZE
#define CPP_MQTT_MAX_STRING_LENGTH MAX_BUFFER_SIZE
#else
#define CPP_MQTT_MAX_STRING_LENGTH 512
#endif
#endif
// Kept for code written against the original name
#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE CPP_MQTT_MAX_STRING_LENGTH
#endif

// static: largest PUBLISH payload received per connection. The client tells the
// broker its Maximum Packet Size based on this, so larger messages are not sent.
#ifndef CPP_MQTT_MAX_PAYLOAD_LENGTH
#define CPP_MQTT_MAX_PAYLOAD_LENGTH 1024
#endif

// static: properties held by one packet or will message
#ifndef CPP_MQTT_MAX_PROPERTIES
#define CPP_MQTT_MAX_PROPERTIES 8
#endif

// static: property objects shared by every packet in the program
#ifndef CPP_MQTT_PROPERTY_POOL_SIZE
#define CPP_MQTT_PROPERTY_POOL_SIZE 16
#endif

// static: outgoing QoS 1/2 messages awaiting acknowledgement, incoming QoS 2
// messages awaiting release, and QoS 0 results awaiting their callback
#ifndef CPP_MQTT_MAX_INFLIGHT
#define CPP_MQTT_MAX_INFLIGHT 8
#endif

// static: topics in one SUBSCRIBE or UNSUBSCRIBE
#ifndef CPP_MQTT_MAX_SUBSCRIPTIONS
#define CPP_MQTT_MAX_SUBSCRIPTIONS 8
#endif

// static: reason codes kept from one SUBACK or UNSUBACK
#ifndef CPP_MQTT_MAX_REASON_CODES
#define CPP_MQTT_MAX_REASON_CODES CPP_MQTT_MAX_SUBSCRIPTIONS
#endif

// static: characters of the broker address passed to connect()
#ifndef CPP_MQTT_MAX_ADDRESS_LENGTH
#define CPP_MQTT_MAX_ADDRESS_LENGTH 64
#endif

// Both modes: bytes buffered on the stack while a packet is written to the
// socket. Packets are streamed in chunks of this size rather than assembled whole.
#ifndef CPP_MQTT_WRITE_CHUNK_SIZE
#define CPP_MQTT_WRITE_CHUNK_SIZE 64
#endif

// Both modes, with a TransmitBuffer: bytes publish(), subscribe() and
// unsubscribe() must leave free, so acknowledgements, pings and DISCONNECT can
// still be sent when the application has filled the buffer
#ifndef CPP_MQTT_TRANSMIT_RESERVE
#define CPP_MQTT_TRANSMIT_RESERVE 32
#endif

/*
 * Memory mode check. Each file that includes the library's headers references a
 * symbol that only a library built in the same mode defines.
 */
#if defined(__GNUC__) && !defined(CPP_MQTT_NO_MODE_CHECK)
#ifdef STATIC_MEMORY
extern "C" const char cpp_mqtt_memory_mode_mismatch_library_must_be_built_with_CPP_MQTT_STATIC_MEMORY;
__attribute__((used, unused)) static const char *const cppMqttMemoryModeCheck =
    &cpp_mqtt_memory_mode_mismatch_library_must_be_built_with_CPP_MQTT_STATIC_MEMORY;
#else
extern "C" const char cpp_mqtt_memory_mode_mismatch_library_must_be_built_without_CPP_MQTT_STATIC_MEMORY;
__attribute__((used, unused)) static const char *const cppMqttMemoryModeCheck =
    &cpp_mqtt_memory_mode_mismatch_library_must_be_built_without_CPP_MQTT_STATIC_MEMORY;
#endif
#endif

#endif /* SRC_CPPMQTTCONFIG */
