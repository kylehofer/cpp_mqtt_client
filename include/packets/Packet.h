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

#ifndef PACKET
#define PACKET

#include <stdint.h>
#include "types/VariableByteInteger.h"
#include "types/EncodedString.h"
#include "Client.h"
#include "ClientInteractor.h"

#define CONNECT_ID (1 << 4)
#define CONNECT_ACKNOWLEDGE_ID (2 << 4)
#define PUBLISH_ID (3 << 4)
#define PUBLISH_ACKNOWLEDGE_ID (4 << 4)
#define PUBLISH_RECEIVED_ID (5 << 4)
#define PUBLISH_RELEASE_ID (6 << 4)
#define PUBLISH_COMPLETE_ID (7 << 4)
#define SUBSCRIBE_ID (8 << 4)
#define SUBSCRIBE_ACKNOWLEDGE_ID (9 << 4)
#define UNSUBSCRIBE_ID (10 << 4)
#define UNSUBSCRIBE_ACKNOWLEDGE_ID (11 << 4)
#define PING_REQUEST_ID (12 << 4)
#define PING_RESPONSE_ID (13 << 4)
#define DISCONNECT_ID (14 << 4)
#define AUTHENTICATION_ID (15 << 4)

namespace PicoMqtt
{
    enum ReasonCode
    {
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

    };

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
        void setFlags(uint8_t);

    public:
        Packet(uint8_t fixedHeaderByte);
        Packet(FixedHeader fixedHeader);
        virtual ~Packet();
        virtual size_t size() = 0;
        virtual bool readFromClient(Client *client, uint32_t &read) = 0;
        virtual size_t pushToClient(Client *client) = 0;
        void setRemainingLength(VariableByteInteger remainingLength);
        void setRemainingLength(uint32_t remainingLength);
        uint8_t getPacketType();
    };

}

#endif /* PACKET */
