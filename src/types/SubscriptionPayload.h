/*
 * File: SubscriptionPayload.h
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

#ifndef SUBSCRIPTIONPAYLOAD
#define SUBSCRIPTIONPAYLOAD

#include <stdint.h>
#include "ClientInteractor.h"
#include "types/EncodedString.h"
#include "types/Common.h"
#include "types/VariableByteInteger.h"

namespace CppMqtt
{
    /**
     * @brief Represents a MQTT 5 Subscription Payload
     * Used for representing a single topic with a set of options for Un/subscribing.
     * Can read and write to a communication client
     */
    class SubscriptionPayload : ClientInteractor
    {
    private:
        EncodedString topic;

    protected:
    public:
        /**
         * @brief Pushes the contents of the Subscription Payload to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t push(PacketBuffer &buffer) override;
        /**
         * @brief Reads data from a client which will then be used to fill in the Subscription Payload
         *
         * @param client The client to read data from
         * @param read The amount of bytes read
         * @return true If more data is required from the client
         * @return false If the class has finished reading data from the client
         */
        virtual bool readFromClient(Client *client, uint32_t &read) override;
        virtual size_t size();

        void setSubscriptionIdentifier(VariableByteInteger value);
        VariableByteInteger getSubscriptionIdentifier();

        void setTopic(EncodedString &topic);
        void setTopic(const char *data, uint16_t length);
    };

}

#endif /* SUBSCRIPTIONPAYLOAD */
