/*
 * File: BigEndianInt.h
 * Project: cpp_mqtt_client
 * Created Date: Saturday April 22nd 2023
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

#ifndef BIGENDIANINT
#define BIGENDIANINT

#include <stdint.h>
#include "Client.h"
#include "ClientInteractor.h"

namespace CppMqtt
{
    /**
     * @brief A Class for ordering Integers in Big Endian Ordering for use with buffers
     *
     * @tparam T int type
     */
    template <class T>
    class BigEndianInt : ClientInteractor
    {
    private:
        //
        union
        {
            uint8_t raw[sizeof(T)];
            T value;
        };

    public:
        BigEndianInt(){};
        /**
         * @brief Constructor so this class can be initialized with the base integer type
         *
         * @param value
         */
        BigEndianInt(T value) : value(value){};

        /**
         * @brief Operator so this class can be treated as the base integer type with assignments
         *
         * @param right
         * @return BigEndianInt&
         */
        BigEndianInt &operator=(const T right)
        {
            this->value = right;
            return *this;
        }

        /**
         * @brief Operator so this class can be treated as the base integer type
         *
         * @return T
         */
        operator T() const
        {
            return value;
        }

        /**
         * @brief Reads from the client using using Big Endian ordering
         *
         * @param client
         * @param read
         * @return true
         * @return false
         */
        bool readFromClient(Client *client, uint32_t &read)
        {
            if (((size_t)client->available()) >= size())
            {
#if BYTE_ORDER == BIG_ENDIAN
                client.read(raw, size());
#else
                for (int i = size() - 1; i >= 0; i--)
                {
                    client->read(&raw[i], 1);
                }
#endif

                read += size();
                return false;
            }
            return true;
        }

        /**
         * @brief Pushes data to the client using Big Endian ordering
         *
         * @param client
         * @return size_t
         */
        size_t push(PacketBuffer &buffer)
        {
#if BYTE_ORDER == BIG_ENDIAN
            buffer.push(raw, size());
#else
            for (int i = size() - 1; i >= 0; i--)
            {
                buffer.push(raw[i]);
            }
#endif

            return size();
        }

        /**
         * @brief Returns the memory size of the integer
         *
         * @return size_t
         */
        inline size_t size()
        {
            return sizeof(T);
        }
    };
}

#endif /* BIGENDIANINT */
