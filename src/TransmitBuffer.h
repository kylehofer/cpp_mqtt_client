/*
 * File: TransmitBuffer.h
 * Project: cpp_mqtt_client
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
 */

#ifndef SRC_TRANSMITBUFFER
#define SRC_TRANSMITBUFFER

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Memory owned by the network client that MqttClient encodes packets
 * into directly, so a packet is written once and sent from there without
 * further copies.
 *
 * Implement it alongside Client in a network client and pass it to
 * MqttClient::setTransmitBuffer(). Without one, MqttClient falls back to
 * Client::write().
 *
 * For each packet MqttClient:
 *  1. checks writable() is at least the packet's size, and does not send
 *     anything if it is not;
 *  2. writes the packet through writePointer(), in as many contiguous spans as
 *     the memory needs (for example either side of a ring buffer's wrap);
 *  3. calls commit() once with the packet's full size.
 * Nothing written before commit() may be sent, so a packet is either sent whole
 * or not at all.
 *
 * TransmitRing<N> (utils/TransmitRing.h) provides the bookkeeping for a ring
 * buffer implementation.
 */
class TransmitBuffer
{
public:
    virtual ~TransmitBuffer() {}

    /**
     * @return size_t Bytes that can be written before the next commit()
     */
    virtual size_t writable() = 0;

    /**
     * @brief Where to write the byte offset bytes past the start of the packet
     * being written (0 for its first byte)
     *
     * @param offset Bytes of the current packet already written
     * @param contiguous Set to the number of bytes that can be written at the
     * returned address
     * @return uint8_t* The address, or NULL if offset is at or past writable()
     */
    virtual uint8_t *writePointer(size_t offset, size_t &contiguous) = 0;

    /**
     * @brief Hands the first length bytes written since the last commit to the
     * network to be sent
     *
     * @return true If the bytes were accepted
     */
    virtual bool commit(size_t length) = 0;
};

#endif /* SRC_TRANSMITBUFFER */
