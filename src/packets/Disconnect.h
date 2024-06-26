/*
 * File: Disconnect.h
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

#ifndef SRC_PACKETS_DISCONNECT
#define SRC_PACKETS_DISCONNECT

#include <stdint.h>
#include "PropertiesPacket.h"
#include "types/VariableByteInteger.h"

namespace CppMqtt
{
    /**
     * @brief Represents a MQTT 5 Disconnect Packet
     *
     */
    class Disconnect : public PropertiesPacket
    {
    private:
        uint8_t state = 0;
        uint8_t reasonCode = 0;

    protected:
    public:
        Disconnect();
        Disconnect(uint8_t flags);
        /**
         * @brief Pushes the contents of the Disconnect Packet to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t push(PacketBuffer &buffer) override;
        /**
         * @brief Reads data from a client which will then be used to fill in the Disconnect Packet
         *
         * @param client The client to read data from
         * @param read The amount of bytes read
         * @return true If more data is required from the client
         * @return false If the class has finished reading data from the client
         */

        virtual bool readFromClient(Client *client, uint32_t &read) override;
        size_t size();

        /**
         * @brief Set the Reason Code
         *
         * @param value
         */
        void setReasonCode(uint8_t value);

        /**
         * @brief Get the Reason Code
         *
         * @param value
         */
        uint8_t getReasonCode();
        /**
         * @brief Validates the packet to the MQTT 5 standards
         *
         * @return true
         * @return false
         */
        virtual bool validate() override;
    };

}

#endif /* SRC_PACKETS_DISCONNECT */
