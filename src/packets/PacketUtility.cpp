
#include "packets/PacketUtility.h"

enum class ReadState
{
    IDENTIFIER_FLAGS,
    PACKET_LENGTH,
    PACKET_CONTENTS
};

namespace PicoMqtt
{
    static Packet *constructPacketFromId(uint8_t identifier)
    {
        switch (identifier & 0xF0) // Strip lower 4 bits
        {
        case CONNECT_ID:
            return (Packet *)new Connect();
        case CONNECT_ACKNOWLEDGE_ID:
            return (Packet *)new ConnectAcknowledge();
        case PUBLISH_ID:
            return (Packet *)new Publish();
        case PUBLISH_ACKNOWLEDGE_ID:
            return (Packet *)new PublishAcknowledge();
        case PUBLISH_RECEIVED_ID:
            return (Packet *)new PublishReceived();
        case PUBLISH_RELEASE_ID:
            return (Packet *)new PublishRelease();
        case PUBLISH_COMPLETE_ID:
            return (Packet *)new PublishComplete();
        case SUBSCRIBE_ID:
            return (Packet *)new Subscribe();
        case SUBSCRIBE_ACKNOWLEDGE_ID:
            return (Packet *)new SubscribeAcknowledge();
        case UNSUBSCRIBE_ID:
            return (Packet *)new Unsubscribe();
        case UNSUBSCRIBE_ACKNOWLEDGE_ID:
            return (Packet *)new UnsubscribeAcknowledge();
        case PING_REQUEST_ID:
            return (Packet *)new PingRequest();
        case PING_RESPONSE_ID:
            return (Packet *)new PingResponse();
        case DISCONNECT_ID:
            return (Packet *)new Disconnect();
        case AUTHENTICATION_ID:
            return (Packet *)new Authentication();
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
        static VariableByteInteger length;
        static Packet *packet;
        while (client->available() > 0)
        {
            switch (state)
            {
            case ReadState::IDENTIFIER_FLAGS:
                client->read(&controlPacket, 1);
                state = ReadState::PACKET_LENGTH;
                packet = constructPacketFromId(controlPacket);
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
