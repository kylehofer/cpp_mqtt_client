/*
 * File: Payload.h
 * Project: cpp_mqtt_client
 * Created Date: Wednesday March 15th 2023
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

#ifndef PAYLOAD
#define PAYLOAD

#include <stdint.h>
#include "ClientInteractor.h"
#include "CppMqttConfig.h"

namespace CppMqtt
{
    /**
     * @brief Represents a MQTT 5 Publish Payloads
     * Used for reading and writing an array of bytes to a communication client
     */
    class Payload : ClientInteractor
    {
    private:
        uint8_t *data = NULL;
        uint32_t length = 0;
        uint32_t bytesRead = 0;
        bool ownership = true;
        bool truncated = false;

    protected:
    public:
        /**
         * With dynamic memory a Payload owns a copy of its bytes, except one made
         * with wrap() or setReference().
         *
         * With STATIC_MEMORY a Payload never owns memory: the data constructor,
         * copies and assignment all refer to the original bytes, which must stay
         * valid while the Payload is used.
         */
        Payload();
#ifndef STATIC_MEMORY
        /**
         * @brief Allocates length bytes. Not available with STATIC_MEMORY.
         */
        Payload(uint32_t length);
#endif
        Payload(void *data, uint32_t length);
        Payload(const Payload &payload);
        ~Payload();

        /**
         * @brief A Payload that refers to data without copying or owning it
         */
        static Payload wrap(void *data, uint32_t length);

        Payload &operator=(const Payload &right);

        uint8_t operator[](int i) const { return data[i]; }
        uint8_t &operator[](int i) { return data[i]; }

        /**
         * @brief Refers to data without copying or owning it, in either memory mode
         */
        void setReference(void *data, uint32_t length);

#ifndef STATIC_MEMORY
        /**
         * @brief Replaces the contents with length newly allocated bytes, for
         * reading into. Not available with STATIC_MEMORY.
         */
        void allocate(uint32_t length);
#endif

        /**
         * @brief Whether a received payload was larger than the receive buffer
         * (STATIC_MEMORY only) and was cut to CPP_MQTT_MAX_PAYLOAD_LENGTH bytes
         */
        bool wasTruncated();
        void setTruncated(bool value);

        uint8_t *getData();
        void setData(void *data, uint32_t length);
        size_t size();
        /**
         * @brief Pushes the contents of the Payload to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t push(PacketBuffer &buffer) override;
        /**
         * @brief Reads data from a client which will then be used to fill in the Payload
         *
         * @param client The client to read data from
         * @param read The amount of bytes read
         * @return true If more data is required from the client
         * @return false If the class has finished reading data from the client
         */
        virtual bool readFromClient(Client *client, uint32_t &read) override;
    };
}
#endif /* PAYLOAD */
