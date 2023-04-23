/*
 * File: Subscription.h
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

#ifndef SUBSCRIPTION
#define SUBSCRIPTION

#include <stdint.h>
#include <vector>
#include "PropertiesPacket.h"
#include "types/SubscriptionPayload.h"

namespace PicoMqtt
{
    /**
     * @brief Represents a base MQTT 5 packet for Un/Subscribe
     * Contains a Variable Header with customziable Flags and Properties
     * Contains a Payload of topics to Un/Subscribe to
     */
    class Subscription : public PropertiesPacket
    {
    private:
        uint8_t state = 0;
        uint16_t packetIdentifier;
        vector<SubscriptionPayload> payloads;

    protected:
        Subscription(uint8_t fixedHeaderByte) : PropertiesPacket(fixedHeaderByte){};

    public:
        size_t size();
        /**
         * @brief Pushes the contents of the Subscription Packet to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t pushToClient(Client *client) override;
        /**
         * @brief Reads data from a client which will then be used to fill in the Subscription Packet
         *
         * @param client The client to read data from
         * @param read The amount of bytes read
         * @return true If more data is required from the client
         * @return false If the class has finished reading data from the client
         */
        virtual bool readFromClient(Client *client, uint32_t *read) override;
    };

}

#endif /* SUBSCRIPTION */
