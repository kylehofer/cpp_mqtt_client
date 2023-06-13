#include "TestTcpClient.h"

#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <poll.h>

#include <chrono>
#include <thread>

#define RETRY_TIMEOUT 5
#define FORWARDER_BUFFER_SIZE 4096
#define LISTENER_BACKLOG 40

using ::chrono::milliseconds;
using ::this_thread::sleep_for;

/**
 * @brief Print out a buffer as hex characters
 *
 * @param buffer
 * @param length
 */
void printBuffer(const void *buffer, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("%x ", ((uint8_t *)buffer)[i] & 0xff);
    }

    printf(" - %ld\n", length);
}

static DataBuffer *initializeDataBuffer(const uint8_t *buffer, size_t length)
{
    DataBuffer *dataBuffer;
    dataBuffer = (DataBuffer *)malloc(sizeof(DataBuffer));

    const uint8_t *data = (const uint8_t *)malloc(length);

    dataBuffer->data = data;
    dataBuffer->position = data;
    dataBuffer->length = length;
    dataBuffer->next = NULL;

    if (buffer != NULL)
    {
        memcpy((void *)dataBuffer->data, buffer, length);
    }
    return dataBuffer;
}

static DataBuffer *initializeDataBuffer(size_t length)
{
    return initializeDataBuffer(NULL, length);
}

int TestTcpClient::connect(const char *host, uint16_t port)
{
    this->port = port;
    return start();
}

size_t TestTcpClient::write(uint8_t data)
{
    return write(&data, 1);
}

size_t TestTcpClient::write(const void *buffer, size_t size)
{

    if (size == 0)
    {
        return 0;
    }

    return ::write(openSocket, buffer, size);
}

void TestTcpClient::flush()
{
    if (writeSize == 0)
    {
        return;
    }

    DataBuffer *temp;
    uint16_t remaining = writeSize;

    void *data = malloc(writeSize);
    char *position = (char *)data;

    while (remaining > 0 && writeQueue != NULL)
    {
        remaining -= writeQueue->length;

        temp = writeQueue;
        writeQueue = writeQueue->next;

        memcpy(position, temp->data, temp->length);

        position += temp->length;

        free((void *)temp->data);
        free(temp);
    }

    ::write(openSocket, data, writeSize);

    writeSize = 0;
}

int TestTcpClient::available()
{
    return availableData;
}

int TestTcpClient::read(void *buffer, size_t size)
{
    char *position = (char *)buffer;
    size_t remaining, dataRead;
    DataBuffer *queue, *temp;

    queue = receiveQueue;

    dataRead = remaining = size < availableData ? size : availableData;

    while (remaining > 0 && queue != NULL)
    {
        if (remaining >= queue->length)
        {
            remaining -= queue->length;

            memcpy(position, queue->position, queue->length);

            position += queue->length;

            temp = queue;

            queue = queue->next;

            free((void *)temp->data);
            free(temp);
            receiveQueue = queue;
        }
        else
        {
            queue->length -= remaining;

            memcpy(position, queue->position, remaining);

            queue->position += remaining;
            remaining = 0;
        }
    }

    availableData -= dataRead;

    return dataRead;
}

int TestTcpClient::start()
{
    return open(this->port);
}

void TestTcpClient::stop()
{
    if (openSocket)
    {
        closeSocket(openSocket);
    }

    auto item = receiveQueue;

    while (item)
    {
        auto temp = item;
        item = item->next;

        free((void *)temp->data);
        free(temp);
    }

    receiveQueue = NULL;

    availableData = 0;
    openSocket = -1;
}

uint8_t TestTcpClient::connected()
{
    int error = 0;
    socklen_t len = sizeof(error);
    int retval = getsockopt(openSocket, SOL_SOCKET, SO_ERROR, &error, &len);

    if (retval != 0)
    {
        /* there was a problem getting the error code */
        fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
        return 0;
    }

    if (error != 0)
    {
        /* socket has a non zero error status */
        fprintf(stderr, "socket error: %s\n", strerror(error));
        return 0;
    }

    return openSocket;
}

void TestTcpClient::sync()
{
    struct pollfd pfd[1];

    pfd[0].fd = openSocket;
    pfd[0].events = POLLIN;

    char buffer[FORWARDER_BUFFER_SIZE];
    int readLength, writeLength, written;

    poll(pfd, 1, RETRY_TIMEOUT);
    if ((pfd[0].revents & (POLLIN)))
    {
        readLength = ::read(openSocket, buffer, FORWARDER_BUFFER_SIZE);
        if (readLength <= 0)
        {
            return;
        }

        DataBuffer *received;
        received = initializeDataBuffer((const uint8_t *)buffer, readLength);

        // memcpy((void *)received->data, buffer, readLength);
        availableData += readLength;

        if (receiveQueue == NULL)
        {
            receiveQueue = received;
        }
        else
        {
            DataBuffer *tail = receiveQueue, *next = tail->next;
            while (next != NULL)
            {
                tail = next;
                next = tail->next;
            }

            tail->next = received;
        }
    }
}

int TestTcpClient::open(int port)
{
    int result;
    struct hostent *forward;
    struct sockaddr_in address;

    forward = gethostbyname("localhost");

    if (forward == NULL)
    {
        printf("Failed to get localhost for output.\n");
        return -1;
    }

    bzero((char *)&address, sizeof(address));
    address.sin_family = AF_INET;
    bcopy((char *)forward->h_addr, (char *)&address.sin_addr.s_addr, forward->h_length);
    address.sin_port = htons(port);

    openSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (openSocket < 0)
    {
        printf("Failed to create Output socket. Port: %d. Error: %s\n", port, strerror(errno));
        return openSocket;
    }

    result = ::connect(openSocket, (struct sockaddr *)&address, sizeof(address));

    if (result != 0)
    {
        printf("Failed to open Output socket. Port: %d. Error: %s\n", port, strerror(errno));
        return result;
    }

    return openSocket;
}

TestTcpClient::~TestTcpClient()
{
    stop();
}

void TestTcpClient::closeSocket(int socket)
{
    char buffer[FORWARDER_BUFFER_SIZE];

    ::close(socket);
}

bool TestTcpClient::portCheck(int port, int maxRetries)
{
    int socketCheck;
    struct sockaddr_in serverAddress;
    struct hostent *server;

    server = gethostbyname("localhost");

    if (server == NULL)
    {
        printf("Failed to get localhost for output.\n");
        return false;
    }

    socketCheck = ::socket(AF_INET, SOCK_STREAM, 0);

    if (socketCheck < 0)
    {
        printf("Failed to open socket for checking port: %d. Error: %s\n", port, strerror(errno));
        return false;
    }

    bzero((char *)&serverAddress, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serverAddress.sin_addr.s_addr,
          server->h_length);

    serverAddress.sin_port = htons(port);

    int retries = 0;

    // Loop until port opens
    while (
        ::connect(socketCheck, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0 &&
        retries++ < maxRetries)
    {
        sleep_for(milliseconds(RETRY_TIMEOUT));
    };

    close(socketCheck);
    return retries <= maxRetries;
}