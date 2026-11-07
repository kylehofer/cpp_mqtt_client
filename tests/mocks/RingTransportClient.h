#ifndef TESTS_MOCKS_RINGTRANSPORTCLIENT
#define TESTS_MOCKS_RINGTRANSPORTCLIENT

#include <stdint.h>
#include <string.h>
#include "SocketClient.h"
#include "TransmitBuffer.h"
#include "utils/TransmitRing.h"

/**
 * @brief A test network client that owns a TransmitRing, standing in for a
 * zero-copy TCP client such as PicoTcpClient. Nothing is sent until deliver()
 * (the network taking bytes) and nothing is freed until acknowledge() (the peer
 * acknowledging them), so tests control both. Never allocates.
 */
template <size_t RING_SIZE>
class RingTransportClient : public Client, public TransmitBuffer
{
private:
    CppMqtt::TransmitRing<RING_SIZE> ring;
    uint8_t wire[8192];
    size_t wireCount = 0;
    uint8_t readBuffer[4096];
    size_t readTotal = 0;
    size_t readCount = 0;
    bool isConnected = false;

public:
    int commits = 0;
    int writes = 0;

    // TransmitBuffer
    size_t writable() override { return ring.writable(); }
    uint8_t *writePointer(size_t offset, size_t &contiguous) override { return ring.writePointer(offset, contiguous); }
    bool commit(size_t length) override
    {
        commits++;
        return ring.commit(length);
    }

    // Client
    int connect(const char *, uint16_t) override
    {
        isConnected = true;
        ring.clear();
        return 1;
    }
    size_t write(uint8_t byte) override { return write(&byte, 1); }
    size_t write(const void *buffer, size_t size) override
    {
        // The copying path also goes through the ring, as PicoTcpClient does
        writes++;
        if (size > ring.writable())
        {
            return 0;
        }
        const uint8_t *source = (const uint8_t *)buffer;
        size_t done = 0;
        while (done < size)
        {
            size_t contiguous = 0;
            uint8_t *destination = ring.writePointer(done, contiguous);
            size_t count = (size - done) < contiguous ? (size - done) : contiguous;
            memcpy(destination, source + done, count);
            done += count;
        }
        ring.commit(size);
        return size;
    }
    int available() override { return (int)(readTotal - readCount); }
    int read(void *buffer, size_t size) override
    {
        if (size > readTotal - readCount)
        {
            size = readTotal - readCount;
        }
        memcpy(buffer, readBuffer + readCount, size);
        readCount += size;
        return (int)size;
    }
    void stop() override { isConnected = false; }
    uint8_t connected() override { return isConnected; }
    void sync() override {}

    /**
     * @brief The network takes every committed byte (in wrap-sized spans)
     */
    void deliver()
    {
        size_t length = 0;
        const uint8_t *span;
        while ((span = ring.unsent(length)) != NULL && length > 0)
        {
            memcpy(wire + wireCount, span, length);
            wireCount += length;
            ring.markSent(length);
        }
    }

    /**
     * @brief The peer acknowledges every delivered byte, freeing the ring
     */
    void acknowledge() { ring.release(ring.unacknowledged()); }

    void deliverAndAcknowledge()
    {
        deliver();
        acknowledge();
    }

    bool feed(const void *data, size_t size)
    {
        if (readTotal + size > sizeof(readBuffer))
        {
            return false;
        }
        memcpy(readBuffer + readTotal, data, size);
        readTotal += size;
        return true;
    }

    const uint8_t *wireData() { return wire; }
    size_t wireLength() { return wireCount; }
    void clearWire() { wireCount = 0; }
    size_t pending() { return ring.pending(); }
};

#endif /* TESTS_MOCKS_RINGTRANSPORTCLIENT */
