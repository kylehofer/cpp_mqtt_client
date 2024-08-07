/*
 * File: BufferData.h
 * Project: cpp_mqtt_client
 * Created Date: Friday March 17th 2023
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

#ifndef BUFFERDATA
#define BUFFERDATA

#include "ClientInteractor.h"
#include "types/BigEndianInt.h"

namespace CppMqtt
{
    /**
     * @brief Represents a raw buffer with a length
     * Used for encoding UTF-8 and binary data for MQTT properties
     */
    class BufferData : ClientInteractor
    {
        uint16_t state;

    public:
        BigEndianInt<uint16_t> length = 0;
#ifdef STATIC_MEMORY
#define MAX_BUFFER_SIZE 512
        char data[MAX_BUFFER_SIZE];
#else
        char *data = NULL;
#endif
        BufferData();
        BufferData(const char *, uint16_t);
        BufferData(const BufferData &);
        BufferData &operator=(const BufferData &right);
        bool operator==(const BufferData &right);
        bool operator!=(const BufferData &right);

        char operator[](int i) const { return data[i]; }
        char &operator[](int i) { return data[i]; }

        ~BufferData();

        /**
         * @brief Pushes the contents of the Buffer to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t push(PacketBuffer &buffer) override;
        size_t size();

        /**
         * @brief Reads data from a client which will then be used to fill in the raw Buffer
         *
         * @param client The client to read data from
         * @param read The amount of bytes read
         * @return true If more data is required from the client
         * @return false If the class has finished reading data from the client
         */
        virtual bool readFromClient(Client *client, uint32_t &read) override;
    };

}

#endif /* BUFFERDATA */
