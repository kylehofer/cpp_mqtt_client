
# cpp_mqtt_client - A C++ Control Loop MQTT Library
This project was developed to provide MQTT5 support to the Pi Pico for the use in other projects I was developing.

## Goal
This projects requirements are based off the requirements of other modules. This was developed for my Sparplug related projects, so the implemented MQTT features are based off those projects. 

## Software Goals
- [ ] Credential Support
- [ ] Auto Reconnect
- [ ] Persistent State


## Building
This library uses cmake for building and dependency management.
The following build flags are supported:
| Flag  | Default | Description |
| ------------- | ------------- |  ------------- |
| BUILD_TARGET | PICO | The build target for the project. Pico requires pico sdk to be available. |
| FETCH_REMOTE | ON | Whether to fetch remote dependencies through cmake. If disabled, the remote dependencies can be put within {PROJECT_ROOT}/external. |
| CPP_MQTT_TESTS | OFF | Whether tests will be compiled. |
| CPP_MQTT_STATIC | ON | Builds as a static library. |
| CPP_MQTT_SHARED | OFF | Builds as a shared library. |
| CPP_MQTT_DYNAMIC_MEMORY | ON | Set to OFF to build without any heap allocation (see below). |

## Static memory
With `CPP_MQTT_DYNAMIC_MEMORY=OFF` (CMake) or `-DCPP_MQTT_STATIC_MEMORY` (PlatformIO `build_flags`), the library never allocates on the heap. Every buffer and container has a fixed size, set by the limits below, and packets are streamed to the socket in small chunks instead of being assembled in memory.

The define changes object layouts, so it must be the same for the library and the application. CMake exports it to every target that links `cpp_mqtt_client`. With PlatformIO, `build_flags` in `platformio.ini` applies to both. A mismatch fails at link time with an undefined reference to `cpp_mqtt_memory_mode_mismatch_*`.

Each limit can be overridden with `-D<NAME>=<value>`, for the library and the application alike:

| Limit | Default | Description |
| ------------- | ------------- |  ------------- |
| CPP_MQTT_MAX_STRING_LENGTH | 512 | Bytes stored per topic, client id or property string. Longer received values are truncated and flagged by `wasTruncated()`. |
| CPP_MQTT_MAX_PAYLOAD_LENGTH | 1024 | Largest received payload. Sent to the broker as the Maximum Packet Size. |
| CPP_MQTT_MAX_PROPERTIES | 8 | Properties held by one packet. Extra received properties are skipped. |
| CPP_MQTT_PROPERTY_POOL_SIZE | 16 | Property objects shared by the whole program. |
| CPP_MQTT_MAX_INFLIGHT | 8 | Unacknowledged QoS 1/2 messages each way. Sent to the broker as the Receive Maximum. `publish()` returns `PUBLISH_FAILED` when full. |
| CPP_MQTT_MAX_SUBSCRIPTIONS | 8 | Topics in one `subscribe()` or `unsubscribe()` call, checked at compile time. |
| CPP_MQTT_MAX_REASON_CODES | 8 | Reason codes kept from one SUBACK or UNSUBACK. |
| CPP_MQTT_MAX_ADDRESS_LENGTH | 64 | Characters in the broker address. |
| CPP_MQTT_WRITE_CHUNK_SIZE | 64 | Stack bytes used to stream a packet to the socket (both modes). |

RAM is dominated by strings: the property pool reserves `CPP_MQTT_PROPERTY_POOL_SIZE` slots, each large enough for two strings. Measured on a 64-bit host, the defaults use about 22 KB (4 KB for the client, 18 KB for the property pool). With `CPP_MQTT_MAX_STRING_LENGTH=128` and `CPP_MQTT_MAX_PAYLOAD_LENGTH=512` this drops to about 7.6 KB.

With static memory a `Payload` never owns memory: copies refer to the original bytes, which must outlive them. `publish()` sends the payload straight from the caller's memory in both modes.

## Sharing the transmit buffer with the network client
By default packets are streamed to `Client::write()` in `CPP_MQTT_WRITE_CHUNK_SIZE` pieces, and the network client copies them again. A network client can instead implement `TransmitBuffer` (`src/TransmitBuffer.h`) and let `MqttClient` encode packets straight into its memory:

```cpp
MqttClient mqtt(&tcpClient, &tcpClient); // Client and TransmitBuffer
```

Each packet is encoded once into the client's memory and committed whole, so a full buffer delays a packet but never sends part of one. `publish()`, `subscribe()` and `unsubscribe()` leave `CPP_MQTT_TRANSMIT_RESERVE` bytes (default 32) free so acknowledgements, pings and DISCONNECT can always be sent; when there is no room, `publish()` returns `PUBLISH_FAILED` and can be retried after `sync()`.

`TransmitRing<N>` (`src/utils/TransmitRing.h`) provides the bookkeeping for a ring buffer whose bytes stay in place until the peer acknowledges them, as zero-copy TCP stacks need.

`PicoTcpClient` (`clients/pico`) uses this with lwIP: packets are encoded into a `PICO_TCP_TRANSMIT_BUFFER_SIZE` ring (default 4096) and sent by `tcp_write` without copying, and received data is read straight from lwIP's pbufs and acknowledged only once read. It allocates nothing on the heap. `PicoMqttClient` owns a `PicoTcpClient` and wires it up this way.

Nagle's algorithm is off by default, so small packets such as PUBACK and PINGREQ go out immediately instead of waiting for earlier data to be acknowledged. Call `setNoDelay(false)` to turn it back on, which batches small packets into fewer transmissions at the cost of latency (useful for battery-powered bulk telemetry), or build with `-DPICO_TCP_NO_DELAY=0` to change the default.

## Dependencies
### Only when building with CPP_MQTT_TESTS
- https://github.com/google/googletest.git
- https://github.com/eclipse/mosquitto.git, only for the end-to-end tests. They are built when libmosquitto (e.g. `libmosquitto-dev`) is found and skipped otherwise.
