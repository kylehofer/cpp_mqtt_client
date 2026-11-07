
#include "packets/PacketUtility.h"

#if defined(DEBUGGING) && DEBUGGING
#define DEBUG(format, ...) printf(format, ##__VA_ARGS__);
#else
#define DEBUG(out, ...)
#endif

namespace CppMqtt
{

#ifdef STATIC_MEMORY
#define PACKET_CREATE(CLASS, IDENTIFIER) \
    (Packet *)new (pool) CLASS(IDENTIFIER)
#else
#define PACKET_CREATE(CLASS, IDENTIFIER) \
    (Packet *)new CLASS(IDENTIFIER)
#endif

    Packet *PacketReader::constructPacketFromId(uint8_t identifier)
    {
#ifdef STATIC_MEMORY
        memset(pool, 0, sizeof(pool));
#endif
        switch (identifier & 0xF0) // Strip lower 4 bits
        {
        case PacketId::CONNECT:
            return PACKET_CREATE(Connect, identifier);
        case PacketId::CONNECT_ACKNOWLEDGE:
            return PACKET_CREATE(ConnectAcknowledge, identifier);
        case PacketId::PUBLISH:
        {
            Publish *publish = (Publish *)PACKET_CREATE(Publish, identifier);
#ifdef STATIC_MEMORY
            publish->setReceiveBuffer(payloadStorage, sizeof(payloadStorage));
#endif
            return publish;
        }
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

    void releasePacket(Packet *packet)
    {
        if (packet == NULL)
        {
            return;
        }
#ifdef STATIC_MEMORY
        packet->~Packet();
#else
        delete packet;
#endif
    }

    PacketReader::~PacketReader()
    {
        // A partially read packet is owned by the reader
        releasePacket(packet);
    }

    Packet *PacketReader::finishPacket()
    {
        state = State::IDENTIFIER_FLAGS;
        length = 0;
        controlPacket = 0;
        Packet *result = packet;
        packet = NULL;
        return result;
    }

    Packet *PacketReader::read(Client *client)
    {
        uint32_t read = 0;

        while (client->available() > 0)
        {
            switch (state)
            {
            case State::IDENTIFIER_FLAGS:
                client->read(&controlPacket, 1);
                state = State::PACKET_LENGTH;
                // NULL for an unrecognised packet type; its contents are skipped below
                packet = constructPacketFromId(controlPacket);
                if (packet != NULL)
                {
                    packet->setFlags(controlPacket);
                }
                // TODO: Malformed packet check
                break;
            case State::PACKET_LENGTH:
                if (!length.readFromClient(client, read))
                {
                    // TODO: Max Packet length check
                    if (length == 0)
                    {
                        if (packet == NULL)
                        {
                            // Unrecognised and empty: nothing to skip
                            finishPacket();
                            break;
                        }
                        return finishPacket();
                    }

                    if (packet == NULL)
                    {
                        DEBUG("Skipping unrecognised packet type: %d\n", controlPacket);
                        skipRemaining = length;
                        state = State::SKIP_CONTENTS;
                        break;
                    }

                    state = State::PACKET_CONTENTS;

                    packet->setRemainingLength(length);
                }
                break;
            case State::SKIP_CONTENTS:
            {
                uint8_t discard[16];
                uint32_t toRead = skipRemaining < sizeof(discard) ? skipRemaining : sizeof(discard);
                if ((uint32_t)client->available() < toRead)
                {
                    toRead = client->available();
                }
                client->read(discard, toRead);
                skipRemaining -= toRead;
                if (skipRemaining == 0)
                {
                    finishPacket();
                }
            }
            break;
            case State::PACKET_CONTENTS:
                if (!packet->readFromClient(client, read))
                {
                    return finishPacket();
                }
                // The packet consumed all it could; wait for more data
                return NULL;

            default:
                break;
            }
        }

        return NULL;
    }

    Packet *readPacketFromClient(Client *client)
    {
        static PacketReader reader;
        return reader.read(client);
    }
}
