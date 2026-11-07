/*
 * File: BufferData.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>

#include "types/BufferData.h"

using namespace CppMqtt;

#define STRING_LENGTH_SIZE sizeof(uint16_t)

enum ReadingState
{
    IDLE = 0,
    LENGTH,
    DATA
};

BufferData::BufferData()
{
    state = IDLE;
}

BufferData::BufferData(const char *string, uint16_t length)
{
    state = IDLE;
#ifdef STATIC_MEMORY
    if (length > CPP_MQTT_MAX_STRING_LENGTH)
    {
        // Keep what fits rather than throwing; exceptions are often disabled on embedded targets
        length = CPP_MQTT_MAX_STRING_LENGTH;
        truncated = true;
    }
#else
    data = (char *)malloc(length);
#endif
    if (length > 0)
    {
        memcpy(data, string, length);
    }
    this->length = length;
}

BufferData::BufferData(const BufferData &source)
{
    state = IDLE;
    truncated = source.truncated;
    length = source.length;
#ifdef STATIC_MEMORY
    memcpy(data, source.data, source.length);
#else
    if (source.data != NULL && source.length > 0)
    {
        data = (char *)malloc(source.length);
        memcpy(data, source.data, source.length);
    }
    else
    {
        data = NULL;
        length = 0;
    }
#endif
}

BufferData &BufferData::operator=(const BufferData &right)
{
    if (&right != this)
    {
        state = IDLE;
        truncated = right.truncated;
        length = right.length;
#ifdef STATIC_MEMORY
        memcpy(data, right.data, right.length);
#else
        if (data != NULL)
        {
            free(data);
            data = NULL;
        }
        if (right.data != NULL && right.length > 0)
        {
            data = (char *)malloc(right.length);
            memcpy(data, right.data, right.length);
        }
        else
        {
            length = 0;
        }
#endif
    }
    return *this;
}

bool BufferData::operator==(const BufferData &right)
{
    return (length == right.length && (length == 0 || memcmp(data, right.data, length) == 0));
}

bool BufferData::operator!=(const BufferData &right)
{
    return !(*this == right);
}

BufferData::~BufferData()
{
#ifndef STATIC_MEMORY
    if (data)
    {
        free(data);
        data = NULL;
    }
#endif
}

size_t BufferData::push(PacketBuffer &buffer)
{
    length.push(buffer);

    if (length > 0)
    {
        buffer.push(data, length);
    }

    return size();
}

size_t BufferData::size()
{
    return length + STRING_LENGTH_SIZE;
}

bool BufferData::readFromClient(Client *client, uint32_t &read)
{
    if (state == IDLE)
    {
        state = LENGTH;
        received = 0;
        truncated = false;
#ifndef STATIC_MEMORY
        if (data != NULL)
        {
            free(data);
            data = NULL;
        }
#endif
    }

    if (state == LENGTH)
    {
        if (length.readFromClient(client, read))
        {
            // The two length bytes have not arrived yet
            return true;
        }

        state = DATA;
        received = 0;
#ifdef STATIC_MEMORY
        // Anything beyond the fixed buffer is read and discarded
        truncated = length > CPP_MQTT_MAX_STRING_LENGTH;
        memset(data, 0, truncated ? CPP_MQTT_MAX_STRING_LENGTH : (uint16_t)length);
#else
        if (length > 0)
        {
            data = (char *)malloc(length);
            memset(data, 0, length);
        }
#endif
    }

    // Read whatever has arrived; the string may span several reads
    while (state == DATA && received < length && client->available() > 0)
    {
        uint32_t toRead = length - received;
        if ((uint32_t)client->available() < toRead)
        {
            toRead = client->available();
        }

        int count;
#ifdef STATIC_MEMORY
        if (received < CPP_MQTT_MAX_STRING_LENGTH)
        {
            if (toRead > (uint32_t)(CPP_MQTT_MAX_STRING_LENGTH - received))
            {
                toRead = CPP_MQTT_MAX_STRING_LENGTH - received;
            }
            count = client->read((uint8_t *)data + received, toRead);
        }
        else
        {
            uint8_t discard[16];
            if (toRead > sizeof(discard))
            {
                toRead = sizeof(discard);
            }
            count = client->read(discard, toRead);
        }
#else
        count = client->read((uint8_t *)data + received, toRead);
#endif
        if (count <= 0)
        {
            break;
        }
        received += count;
        read += count;
    }

    if (state == DATA && received >= length)
    {
#ifdef STATIC_MEMORY
        if (truncated)
        {
            length = CPP_MQTT_MAX_STRING_LENGTH;
        }
#endif
        state = IDLE;
        return false;
    }

    return true;
}

bool BufferData::wasTruncated()
{
    return truncated;
}
