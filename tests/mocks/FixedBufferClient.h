#ifndef TESTS_MOCKS_FIXEDBUFFERCLIENT
#define TESTS_MOCKS_FIXEDBUFFERCLIENT

#include <stdint.h>
#include <string.h>
#include "SocketClient.h"

/**
 * @brief A test Client backed by fixed arrays, so it never allocates. Used where
 * the test must not count the mock's own allocations.
 */
class FixedBufferClient : public Client
{
private:
    uint8_t readBuffer[4096];
    size_t readTotal = 0;
    size_t readCount = 0;
    uint8_t writeBuffer[4096];
    size_t writeCount = 0;
    bool isConnected = false;

public:
    int connect(const char *, uint16_t) override
    {
        isConnected = true;
        return 1;
    }
    size_t write(uint8_t byte) override { return write(&byte, 1); }
    size_t write(const void *buffer, size_t size) override
    {
        if (writeCount + size > sizeof(writeBuffer))
        {
            return 0;
        }
        memcpy(writeBuffer + writeCount, buffer, size);
        writeCount += size;
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
     * @brief Queues bytes for the client to read
     */
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
    size_t written() { return writeCount; }
    const uint8_t *writtenData() { return writeBuffer; }
};

#endif /* TESTS_MOCKS_FIXEDBUFFERCLIENT */
