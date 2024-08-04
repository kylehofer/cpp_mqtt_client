
#include "packets/PacketUtility.h"

enum class ReadState
{
    IDENTIFIER_FLAGS,
    PACKET_LENGTH,
    PACKET_CONTENTS
};

#ifdef DEBUGGING
#define DEBUG(format, ...) printf(format, ##__VA_ARGS__);
#else
#define DEBUG(out, ...)
#endif

namespace CppMqtt
{

#ifdef STATIC_MEMORY
    union ALL_PACKETS
    {
        Authentication b;
        Connect c;
        ConnectAcknowledge d;
        Disconnect e;
        PingRequest f;
        PingResponse g;
        Publish h;
        PublishAcknowledge i;
        PublishComplete j;
        PublishReceived k;
        PublishRelease l;
        Subscribe n;
        SubscribeAcknowledge o;
        Unsubscribe p;
        UnsubscribeAcknowledge q;
    };
#define MAX_PACKET_SIZE sizeof(ALL_PACKETS)
    static uint8_t PACKET_MEMORY_POOL[MAX_PACKET_SIZE];
#define PACKET_CREATE(CLASS, IDENTIFIER) \
    (Packet *)new (PACKET_MEMORY_POOL) CLASS(IDENTIFIER)
#else
#define PACKET_CREATE(CLASS, IDENTIFIER) \
    (Packet *)new CLASS(IDENTIFIER)
#endif

    static Packet *constructPacketFromId(uint8_t identifier)
    {
#ifdef STATIC_MEMORY
        memset(PACKET_MEMORY_POOL, 0, MAX_PACKET_SIZE);
#endif
        switch (identifier & 0xF0) // Strip lower 4 bits
        {
        case PacketId::CONNECT:
            return PACKET_CREATE(Connect, identifier);
        case PacketId::CONNECT_ACKNOWLEDGE:
            return PACKET_CREATE(ConnectAcknowledge, identifier);
        case PacketId::PUBLISH:
            return PACKET_CREATE(Publish, identifier);
        case PacketId::PUBLISH_ACKNOWLEDGE:
            return PACKET_CREATE(PublishAcknowledge, identifier);
        case PacketId::PUBLISH_RECEIVED:
            return PACKET_CREATE(PublishReceived, identifier);
        case PacketId::PUBLISH_RELEASE:
            return PACKET_CREATE(PublishRelease, identifier);
        case PacketId::PUBLISH_COMPLETE:
            return PACKET_CREATE(PublishComplete, identifier);
        case PacketId::SUBSCRIBE:
            return PACKET_CREATE(Subscribe, identifier);
        case PacketId::SUBSCRIBE_ACKNOWLEDGE:
            return PACKET_CREATE(SubscribeAcknowledge, identifier);
        case PacketId::UNSUBSCRIBE:
            return PACKET_CREATE(Unsubscribe, identifier);
        case PacketId::UNSUBSCRIBE_ACKNOWLEDGE:
            return PACKET_CREATE(UnsubscribeAcknowledge, identifier);
        case PacketId::PING_REQUEST:
            return PACKET_CREATE(PingRequest, identifier);
        case PacketId::PING_RESPONSE:
            return PACKET_CREATE(PingResponse, identifier);
        case PacketId::DISCONNECT:
            return PACKET_CREATE(Disconnect, identifier);
        case PacketId::AUTHENTICATION:
            return PACKET_CREATE(Authentication, identifier);
        default:
            break;
        }
        return NULL;
    }

    Packet *readPacketFromClient(Client *client)
    {
        uint32_t read = 0;
        static ReadState state = ReadState::IDENTIFIER_FLAGS;
        static uint8_t controlPacket;
        static VariableByteInteger length = 0;
        static Packet *packet;

        while (client->available() > 0)
        {
            switch (state)
            {
            case ReadState::IDENTIFIER_FLAGS:
                client->read(&controlPacket, 1);
                state = ReadState::PACKET_LENGTH;
                packet = constructPacketFromId(controlPacket);
                packet->setFlags(controlPacket);
                // TODO: Malformed packet check
                break;
            case ReadState::PACKET_LENGTH:
                if (!length.readFromClient(client, read))
                {
                    // TODO: Max Packet length check
                    if (length == 0)
                    {
                        state = ReadState::IDENTIFIER_FLAGS;
                        length = 0;
                        controlPacket = 0;
                        {
                            Packet *result = packet;
                            packet = NULL;
                            return result;
                        }
                    }

                    state = ReadState::PACKET_CONTENTS;

                    packet->setRemainingLength(length);
                }
                break;
            case ReadState::PACKET_CONTENTS:
                if (!packet->readFromClient(client, read))
                {
                    state = ReadState::IDENTIFIER_FLAGS;
                    length = 0;
                    controlPacket = 0;
                    {
                        Packet *result = packet;
                        packet = NULL;
                        return result;
                    }
                }
                break;

            default:
                break;
            }
        }

        return NULL;
    }
}
