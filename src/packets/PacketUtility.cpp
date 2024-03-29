
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
    static Packet *constructPacketFromId(uint8_t identifier)
    {
        switch (identifier & 0xF0) // Strip lower 4 bits
        {
        case PacketId::PacketId::CONNECT:
            return (Packet *)new Connect(identifier);
        case PacketId::CONNECT_ACKNOWLEDGE:
            return (Packet *)new ConnectAcknowledge(identifier);
        case PacketId::PUBLISH:
            return (Packet *)new Publish(identifier);
        case PacketId::PUBLISH_ACKNOWLEDGE:
            return (Packet *)new PublishAcknowledge(identifier);
        case PacketId::PUBLISH_RECEIVED:
            return (Packet *)new PublishReceived(identifier);
        case PacketId::PUBLISH_RELEASE:
            return (Packet *)new PublishRelease(identifier);
        case PacketId::PUBLISH_COMPLETE:
            return (Packet *)new PublishComplete(identifier);
        case PacketId::SUBSCRIBE:
            return (Packet *)new Subscribe(identifier);
        case PacketId::SUBSCRIBE_ACKNOWLEDGE:
            return (Packet *)new SubscribeAcknowledge(identifier);
        case PacketId::UNSUBSCRIBE:
            return (Packet *)new Unsubscribe(identifier);
        case PacketId::UNSUBSCRIBE_ACKNOWLEDGE:
            return (Packet *)new UnsubscribeAcknowledge(identifier);
        case PacketId::PING_REQUEST:
            return (Packet *)new PingRequest(identifier);
        case PacketId::PING_RESPONSE:
            return (Packet *)new PingResponse(identifier);
        case PacketId::DISCONNECT:
            return (Packet *)new Disconnect(identifier);
        case PacketId::AUTHENTICATION:
            return (Packet *)new Authentication(identifier);
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
