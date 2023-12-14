/*
 * File: Publish.h
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

#ifndef PUBLISH
#define PUBLISH

#include <stdint.h>
#include "PropertiesPacket.h"
#include "types/Payload.h"
#include "types/Common.h"

namespace PicoMqtt
{
    /**
     * @brief Represents a MQTT 5 Publish Packet
     * Contains a Variable Header with customziable Flags and Properties
     * Contains a Payload of bytes
     */
    class Publish : public PropertiesPacket
    {
    private:
        uint8_t state = 0;
        EncodedString topic;

        uint16_t packetIdentifier = 0;
        Payload payload;

    protected:
    public:
        Publish();
        Publish(uint8_t flags);
        size_t size();
        /**
         * @brief Pushes the contents of the Publish Packet to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t push(PacketBuffer &buffer) override;
        /**
         * @brief Reads data from a client which will then be used to fill in the Publish Packet
         *
         * @param client The client to read data from
         * @param read The amount of bytes read
         * @return true If more data is required from the client
         * @return false If the class has finished reading data from the client
         */
        virtual bool readFromClient(Client *client, uint32_t &read) override;
        EncodedString &getTopic();
        void setTopic(const char *data, uint32_t length);
        void setTopic(EncodedString value);
        Payload &getPayload();
        void setPayload(void *data, uint32_t length);
        void setPayload(Payload value);
        uint16_t getPacketIdentifier();
        void setPacketIdentifier(uint16_t packetIdentifier);
        void setQos(QoS value);
        QoS getQos();
        /**
         * @brief Validates the packet to the MQTT 5 standards
         *
         * @return true
         * @return false
         */
        virtual bool validate() override;
    };

}

#endif /* PUBLISH */
