/*
 * File: PacketUtility.h
 * Project: cpp_mqtt_client
 * Created Date: Thursday March 16th 2023
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
 *
 * HISTORY:
 */

/**
 * @brief Utility file for handling Packets
 *
 */

#ifndef SRC_PACKETS_PACKETUTILITY
#define SRC_PACKETS_PACKETUTILITY

#include "CppMqttConfig.h"
#include "Packet.h"
#include "Authentication.h"
#include "Connect.h"
#include "ConnectAcknowledge.h"
#include "Disconnect.h"
#include "PingRequest.h"
#include "PingResponse.h"
#include "Publish.h"
#include "PublishAcknowledge.h"
#include "PublishComplete.h"
#include "PublishReceived.h"
#include "PublishRelease.h"
#include "Subscribe.h"
#include "SubscribeAcknowledge.h"
#include "Unsubscribe.h"
#include "UnsubscribeAcknowledge.h"
#include "SocketClient.h"

namespace CppMqtt
{
#ifdef STATIC_MEMORY
    /**
     * @brief Storage large enough for any packet a PacketReader can return.
     * Only used for its size and alignment; never constructed.
     */
    union AllPackets
    {
        Authentication authentication;
        Connect connect;
        ConnectAcknowledge connectAcknowledge;
        Disconnect disconnect;
        PingRequest pingRequest;
        PingResponse pingResponse;
        Publish publish;
        PublishAcknowledge publishAcknowledge;
        PublishComplete publishComplete;
        PublishReceived publishReceived;
        PublishRelease publishRelease;
        Subscribe subscribe;
        SubscribeAcknowledge subscribeAcknowledge;
        Unsubscribe unsubscribe;
        UnsubscribeAcknowledge unsubscribeAcknowledge;
        AllPackets() {}
        ~AllPackets() {}
    };
#endif

    /**
     * @brief Incrementally reads MQTT packets from one connection.
     * Holds the parse state for that connection, so each connection needs its
     * own reader. Data may arrive in any number of pieces; read() returns NULL
     * until a whole packet has been received.
     *
     * With STATIC_MEMORY the returned packet lives inside this reader, so it must
     * be released with releasePacket() before the next call to read().
     */
    class PacketReader
    {
    private:
        enum class State : uint8_t
        {
            IDENTIFIER_FLAGS,
            PACKET_LENGTH,
            PACKET_CONTENTS,
            SKIP_CONTENTS
        };

        State state = State::IDENTIFIER_FLAGS;
        uint8_t controlPacket = 0;
        VariableByteInteger length = 0;
        Packet *packet = NULL;
        uint32_t skipRemaining = 0;

#ifdef STATIC_MEMORY
        alignas(AllPackets) uint8_t pool[sizeof(AllPackets)];
        // Received PUBLISH payloads are stored here
        uint8_t payloadStorage[CPP_MQTT_MAX_PAYLOAD_LENGTH];
#endif

        Packet *constructPacketFromId(uint8_t identifier);
        Packet *finishPacket();

    public:
        PacketReader() = default;
        PacketReader(const PacketReader &) = delete;
        PacketReader &operator=(const PacketReader &) = delete;
        ~PacketReader();

        /**
         * @brief Reads as much of the next packet as is available
         *
         * @param client The client to read data from
         * @return Packet* The complete packet, or NULL if more data is needed.
         * Release it with releasePacket().
         */
        Packet *read(Client *client);
    };

    /**
     * @brief Attempts to read a packet from the client using a single shared
     * PacketReader. Only suitable when there is one connection; use a
     * PacketReader per connection otherwise.
     *
     * @return Packet* The processed packet, NULL if a complete packet has not been receieved.
     * Release it with releasePacket().
     */
    Packet *readPacketFromClient(Client *);

    /**
     * @brief Releases a packet returned by a PacketReader or readPacketFromClient.
     * With STATIC_MEMORY the packet lives inside its reader, so only its destructor
     * is run; otherwise it is deleted. Use this rather than delete.
     *
     * @param packet The packet to release, may be NULL
     */
    void releasePacket(Packet *packet);
}

#endif /* SRC_PACKETS_PACKETUTILITY */
