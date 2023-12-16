#ifndef PROPERTIESPACKET
#define PROPERTIESPACKET

#include "Packet.h"
#include "PicoMqttProperties.h"

namespace PicoMqtt
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

#endif /* PROPERTIESPACKET */
