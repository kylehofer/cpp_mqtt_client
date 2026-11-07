/*
 * File: TransmitRing.h
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

#ifndef SRC_UTILS_TRANSMITRING
#define SRC_UTILS_TRANSMITRING

#include <stddef.h>
#include <stdint.h>

namespace CppMqtt
{
    /**
     * @brief A fixed ring of N bytes for data waiting to be sent and acknowledged,
     * for network clients that send without copying (for example lwIP's
     * tcp_write without TCP_WRITE_FLAG_COPY, which needs the bytes to stay
     * unchanged until they are acknowledged).
     *
     * Bytes move through three stages, oldest first:
     *   acknowledged <- released by release()
     *   [ sent: handed to the network, awaiting acknowledgement ]
     *   [ unsent: committed, not yet handed to the network ]
     *   [ free: writable, the next packet is written here ]
     *
     * The writer side (writable, writePointer, commit) matches TransmitBuffer.
     * The network side uses unsent(), markSent() and release().
     */
    template <size_t N>
    class TransmitRing
    {
        static_assert(N > 0, "TransmitRing needs at least one byte");

    private:
        uint8_t data[N];
        // Running byte counts; positions in data are these modulo N
        size_t releasedCount = 0;
        size_t sentCount = 0;
        size_t committedCount = 0;

    public:
        /**
         * @return size_t Free bytes for the next packet
         */
        size_t writable() const
        {
            return N - (committedCount - releasedCount);
        }

        /**
         * @brief Contiguous free space offset bytes past the end of committed data
         *
         * @return uint8_t* The address, or NULL with contiguous 0 if offset is not
         * below writable()
         */
        uint8_t *writePointer(size_t offset, size_t &contiguous)
        {
            size_t free = writable();
            if (offset >= free)
            {
                contiguous = 0;
                return NULL;
            }
            size_t index = (committedCount + offset) % N;
            size_t untilWrap = N - index;
            size_t remaining = free - offset;
            contiguous = untilWrap < remaining ? untilWrap : remaining;
            return data + index;
        }

        /**
         * @brief Marks length written bytes as ready to send
         *
         * @return false If length exceeds the free space; nothing is committed
         */
        bool commit(size_t length)
        {
            if (length > writable())
            {
                return false;
            }
            committedCount += length;
            return true;
        }

        /**
         * @brief The oldest committed bytes not yet handed to the network, as one
         * contiguous span (data that wraps is returned in two calls)
         *
         * @param length Set to the span's length, 0 if nothing is waiting
         */
        const uint8_t *unsent(size_t &length) const
        {
            size_t waiting = committedCount - sentCount;
            if (waiting == 0)
            {
                length = 0;
                return NULL;
            }
            size_t index = sentCount % N;
            size_t untilWrap = N - index;
            length = waiting < untilWrap ? waiting : untilWrap;
            return data + index;
        }

        /**
         * @brief Records that length bytes from unsent() were handed to the network
         */
        void markSent(size_t length)
        {
            size_t waiting = committedCount - sentCount;
            sentCount += (length < waiting) ? length : waiting;
        }

        /**
         * @brief Frees the oldest length sent bytes once the network has
         * acknowledged them
         */
        void release(size_t length)
        {
            size_t inFlight = sentCount - releasedCount;
            releasedCount += (length < inFlight) ? length : inFlight;
        }

        /**
         * @return size_t Bytes handed to the network and not yet acknowledged
         */
        size_t unacknowledged() const { return sentCount - releasedCount; }

        /**
         * @return size_t Bytes committed and not yet acknowledged
         */
        size_t pending() const { return committedCount - releasedCount; }

        /**
         * @brief Discards everything, for a connection that has gone away
         */
        void clear()
        {
            releasedCount = sentCount = committedCount = 0;
        }

        static constexpr size_t capacity() { return N; }
    };
}

#endif /* SRC_UTILS_TRANSMITRING */
