/*
 * File: ConnectAcknowledge.h
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

#ifndef CONNECTACKNOWLEDGE
#define CONNECTACKNOWLEDGE

#include <stdint.h>
#include "PropertiesPacket.h"
#include "types/EncodedString.h"
#include "types/BinaryData.h"
#include "types/VariableByteInteger.h"

namespace PicoMqtt
{
    typedef union
    {
        struct
        {
            uint8_t reserved : 7;
            uint8_t session : 1;
            uint8_t reasonCode;
        };
        uint16_t data;
    } ConnackHeader;

    /**
     * @brief Represents a MQTT 5 Connect Acknowledge Packet
     *
     */
    class ConnectAcknowledge : public PropertiesPacket
    {
    private:
        uint8_t state = 0;
        ConnackHeader header;

    protected:
    public:
        ConnectAcknowledge();
        size_t size() { return 0; };
        /**
         * @brief Pushes the contents of the Connect Acknowledge to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t pushToClient(Client *client) override { return 0; };
        /**
         * @brief Reads data from a client which will then be used to fill in the Connect Acknowledge Packet
         *
         * @param client The client to read data from
         * @param read The amount of bytes read
         * @return true If more data is required from the client
         * @return false If the class has finished reading data from the client
         */
        virtual bool readFromClient(Client *client, uint32_t *read) override;
        uint32_t getSessionExpiryInterval();
        uint16_t getReceiveMaximum();
        uint8_t getMaximumQoS();
        bool getRetainAvailable();
        uint32_t getMaximumPacketSize();
        EncodedString getAssignedClientIdentifier();
        uint16_t getTopicAliasMaximum();
        EncodedString getReasonString();
        bool getWildcardSubscriptionAvailable();
        bool getSubscriptionIdentifiersAvailable();
        bool getSharedSubscriptionAvailable();
        uint16_t getServerKeepAlive();
        EncodedString getResponseInformation();
        EncodedString getServerReference();
        EncodedString getAuthenticationMethod();
        BinaryData getAuthenticationData();
        uint8_t getReasonCode();
    };

}

#endif /* CONNECTACKNOWLEDGE */
