/*
 * File: Packet.h
 * Project: cpp_mqtt_client
 * Created Date: Monday February 27th 2023
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

#ifndef SRC_PACKETS_PACKET
#define SRC_PACKETS_PACKET

#include <stdint.h>
#include "types/VariableByteInteger.h"
#include "types/EncodedString.h"
#include "Client.h"
#include "ClientInteractor.h"
#include "utils/enum.h"

#define HEADER_BYTES_MASK 0xF

namespace PicoMqtt
{

    BETTER_ENUM(PacketId, int,
                CONNECT = (1 << 4),
                CONNECT_ACKNOWLEDGE = (2 << 4),
                PUBLISH = (3 << 4),
                PUBLISH_ACKNOWLEDGE = (4 << 4),
                PUBLISH_RECEIVED = (5 << 4),
                PUBLISH_RELEASE = (6 << 4),
                PUBLISH_COMPLETE = (7 << 4),
                SUBSCRIBE = (8 << 4),
                SUBSCRIBE_ACKNOWLEDGE = (9 << 4),
                UNSUBSCRIBE = (10 << 4),
                UNSUBSCRIBE_ACKNOWLEDGE = (11 << 4),
                PING_REQUEST = (12 << 4),
                PING_RESPONSE = (13 << 4),
                DISCONNECT = (14 << 4),
                AUTHENTICATION = (15 << 4))

    BETTER_ENUM(ReasonCode, uint8_t,
                SUCCESS = 0,                                  // CONNACK, PUBACK, PUBREC, PUBREL, PUBCOMP, UNSUBACK, AUTH
                NORMAL_DISCONNECTION = 0,                     // Disconnect
                GRANTED_QOS_0 = 0,                            // SUBACK
                GRANTED_QOS_1 = 1,                            // SUBACK
                GRANTED_QOS_2 = 2,                            // SUBACK
                DISCONNECT_WITH_WILL_MESSAGE = 4,             // DISCONNECT
                NO_MATCHING_SUBSCRIBERS = 16,                 // PUBACK, PUBREC
                NO_SUBSCRIPTION_FOUND = 17,                   // UNSUBACK
                CONTINUE_AUTHENTICATION = 24,                 // AUTH
                RE_AUTHENTICATE = 25,                         // AUTH
                UNSPECIFIED_ERROR = 128,                      // CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
                MALFORMED_PACKET = 129,                       // CONNACK, DISCONNECT
                PROTOCOL_ERROR = 130,                         // CONNACK, DISCONNECT
                IMPLEMENTATION_SPECIFIC_ERROR = 131,          // CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
                UNSUPPORTED_PROTOCOL_VERSION = 132,           // CONNACK
                CLIENT_IDENTIFIER_NOT_VALID = 133,            // CONNACK
                BAD_USER_NAME_OR_PASSWORD = 134,              // CONNACK
                NOT_AUTHORIZED = 135,                         // CONNACK, PUBACK, PUBREC, SUBACK, UNSUBACK, DISCONNECT
                SERVER_UNAVAILABLE = 136,                     // CONNACK
                SERVER_BUSY = 137,                            // CONNACK, DISCONNECT
                BANNED = 138,                                 // CONNACK
                SERVER_SHUTTING_DOWN = 139,                   // DISCONNECT
                BAD_AUTHENTICATION_METHOD = 140,              // CONNACK, DISCONNECT
                KEEP_ALIVE_TIMEOUT = 141,                     // DISCONNECT
                SESSION_TAKEN_OVER = 142,                     // DISCONNECT
                TOPIC_FILTER_INVALID = 143,                   // SUBACK, UNSUBACK, DISCONNECT
                TOPIC_NAME_INVALID = 144,                     // CONNACK, PUBACK, PUBREC, DISCONNECT
                PACKET_IDENTIFIER_IN_USE = 145,               // PUBACK, PUBREC, SUBACK, UNSUBACK
                PACKET_IDENTIFIER_NOT_FOUND = 146,            // PUBREL, PUBCOMP
                RECEIVE_MAXIMUM_EXCEEDED = 147,               // DISCONNECT
                TOPIC_ALIAS_INVALID = 148,                    // DISCONNECT
                PACKET_TOO_LARGE = 149,                       // CONNACK, DISCONNECT
                MESSAGE_RATE_TOO_HIGH = 150,                  // DISCONNECT
                QUOTA_EXCEEDED = 151,                         // CONNACK, PUBACK, PUBREC, SUBACK, DISCONNECT
                ADMINISTRATIVE_ACTION = 152,                  // DISCONNECT
                PAYLOAD_FORMAT_INVALID = 153,                 // CONNACK, PUBACK, PUBREC, DISCONNECT
                RETAIN_NOT_SUPPORTED = 154,                   // CONNACK, DISCONNECT
                QOS_NOT_SUPPORTED = 155,                      // CONNACK, DISCONNECT
                USE_ANOTHER_SERVER = 156,                     // CONNACK, DISCONNECT
                SERVER_MOVED = 157,                           // CONNACK, DISCONNECT
                SHARED_SUBSCRIPTIONS_NOT_SUPPORTED = 158,     // SUBACK, DISCONNECT
                CONNECTION_RATE_EXCEEDED = 159,               // CONNACK, DISCONNECT
                MAXIMUM_CONNECT_TIME = 160,                   // DISCONNECT
                SUBSCRIPTION_IDENTIFIERS_NOT_SUPPORTED = 161, // SUBACK, DISCONNECT
                WILDCARD_SUBSCRIPTIONS_NOT_SUPPORTED = 162    // SUBACK, DISCONNECT
    )

#define PROTOCOL_NAME_LENGTH 7
    const uint8_t PROTOCOL_NAME[] = {0, 0x4, 'M', 'Q', 'T', 'T', 0x5};

    union FixedHeader
    {
        struct
        {
            uint8_t packetType : 4;
            uint8_t flags : 4;
        };
        uint8_t data;
    };

    /**
     * @brief Represents a MQTT 5 Control Packet
     * Packets can Read and/or Write to a communication client
     * All packets contain a Fixed Header, however all other data is Packet dependant
     */
    class Packet : public virtual ClientInteractor
    {
    private:
        FixedHeader fixedHeader;
        uint8_t state = 0;
        VariableByteInteger remainingLength;
        uint32_t bytesRead = 0;

    protected:
        uint8_t getFixedHeaderFlags();
        uint32_t getRemainingLength();
        void readBytes(uint32_t count);
        bool dataRemaining();
        bool isValid();

    public:
        void setFlags(uint8_t);
        Packet(uint8_t fixedHeaderByte);
        Packet(FixedHeader fixedHeader);
        virtual ~Packet();
        virtual size_t size() = 0;
        virtual bool readFromClient(Client *client, uint32_t &read) = 0;
        virtual size_t push(PacketBuffer &buffer) = 0;
        size_t totalSize();
        void write(Client *client);
        void setRemainingLength(VariableByteInteger remainingLength);
        void setRemainingLength(uint32_t remainingLength);
        uint8_t getPacketType();
        /**
         * @brief Validates the packet to the MQTT 5 standards
         *
         * @return true
         * @return false
         */
        virtual bool validate() = 0;
    };
}

#endif /* SRC_PACKETS_PACKET */
