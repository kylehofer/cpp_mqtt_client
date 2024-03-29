#ifndef TESTS_MOCKS_MOCKCLIENT
#define TESTS_MOCKS_MOCKCLIENT

#include "Client.h"
#include <stdlib.h>
#include <stdint.h>

class MockClient : Client
{
private:
    char *readBuffer = nullptr;
    char *writeBuffer = nullptr;
    size_t readTotal = 0;
    size_t writeTotal = 0;
    size_t readCount = 0;
    size_t writeCount = 0;
    bool isConnected = false;

public:
    MockClient(){};
    ~MockClient()
    {
        if (readBuffer)
        {
            free(readBuffer);
        }

        if (writeBuffer)
        {
            free(writeBuffer);
        }
    };

    int connect(const char *host, uint16_t port);
    size_t write(uint8_t);
    size_t write(const void *buffer, size_t size);
    int available();
    int read(void *buffer, size_t size);
    void stop();
    uint8_t connected();

    bool resizeReadBuffer(size_t);
    bool resizeWriteBuffer(size_t);
    void clearWriteBuffer();

    char *getReadBuffer();
    char *getWriteBuffer();

    bool pushToReadBuffer(void *buffer, size_t size);

    void setIsConnected(bool connected);

    size_t written();

    void sync();
};

#endif /* TESTS_MOCKS_MOCKCLIENT */
