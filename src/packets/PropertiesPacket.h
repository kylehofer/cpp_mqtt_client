#ifndef SRC_PACKETS_PROPERTIESPACKET
#define SRC_PACKETS_PROPERTIESPACKET

#include "Packet.h"
#include "MqttProperties.h"

namespace CppMqtt
{

    /**
     * @brief Represents a MQTT 5 Control Packet
     * Packets can Read and/or Write to a communication client
     * All packets contain a Fixed Header, however all other data is Packet dependant
     */
    class PropertiesPacket : public Packet
    {
    private:
    protected:
        Properties properties;

    public:
        PropertiesPacket(uint8_t fixedHeaderByte) : Packet(fixedHeaderByte){};
        PropertiesPacket(FixedHeader fixedHeader) : Packet(fixedHeader){};
        EncodedString getUserProperty(EncodedString key);
        EncodedString getUserProperty(const char *key, uint32_t keyLength);
    };

}

#endif /* SRC_PACKETS_PROPERTIESPACKET */
