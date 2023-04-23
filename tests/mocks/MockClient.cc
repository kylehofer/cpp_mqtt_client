/*
 * File: MockClient.cc
 * Project: cpp_mqtt_client
 * Created Date: Sunday April 23rd 2023
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

#include "MockClient.h"
#include <string.h>
#include <stdio.h>

int MockClient::connect(const char *host, uint16_t port)
{
    return 1;
}

size_t MockClient::write(uint8_t byte)
{
    return write(&byte, 1);
}

size_t MockClient::write(const void *buffer, size_t size)
{
    if ((writeCount + size) > writeTotal)
    {

        size_t required = (writeCount + size) - writeTotal;

        char *temp = writeBuffer;

        writeBuffer = (char *)malloc(writeTotal + required);

        if (temp != NULL)
        {
            memcpy(writeBuffer, temp, writeCount);
            free(temp);
        }

        writeTotal += required;
    }

    memcpy(writeBuffer + writeCount, buffer, size);

    writeCount += size;

    return size;
}

int MockClient::available()
{
    return readTotal - readCount;
}

int MockClient::read(void *buffer, size_t size)
{
    memcpy(buffer, (readBuffer + readCount), size);
    readCount += size;
    return size;
}

void MockClient::stop()
{
}

uint8_t MockClient::connected()
{
    return isConnected;
}

bool MockClient::resizeReadBuffer(size_t size)
{
    if (readBuffer != NULL)
    {
        free(readBuffer);
    }

    readTotal = size;
    readCount = 0;

    if (size > 0)
    {
        return (readBuffer = (char *)malloc(size)) != NULL;
    }

    return true;
}

bool MockClient::resizeWriteBuffer(size_t size)
{
    if (writeBuffer != NULL)
    {
        free(writeBuffer);
    }
    writeTotal = size;
    writeCount = 0;

    if (size > 0)
    {
        return (writeBuffer = (char *)malloc(size)) != NULL;
    }

    return true;
}

void MockClient::clearWriteBuffer()
{
    resizeWriteBuffer(0);
    writeBuffer = NULL;
}

char *MockClient::getReadBuffer()
{
    return readBuffer;
}

char *MockClient::getWriteBuffer()
{
    return writeBuffer;
}

bool MockClient::pushToReadBuffer(void *buffer, size_t size)
{
    char *temp = NULL;

    if (readBuffer != NULL)
    {
        temp = readBuffer;
    }

    readBuffer = (char *)malloc(readTotal + size);

    if (temp != NULL)
    {
        memcpy(readBuffer, temp, readTotal);
        free(temp);
    }

    memcpy(readBuffer + readTotal, buffer, size);

    readTotal += size;

    return (readBuffer != NULL);
}

void MockClient::setIsConnected(bool connected)
{
    isConnected = connected;
}

size_t MockClient::written()
{
    return writeCount;
}
