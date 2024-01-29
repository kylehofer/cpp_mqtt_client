
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
| MQTT_ENABLE_TESTING | OFF | Whether tests will be compiled. Forcibly disabled when building for PICO |

## Dependencies
### Only when building with MQTT_ENABLE_TESTING
- https://github.com/eclipse/mosquitto.git
- https://github.com/google/googletest.git
