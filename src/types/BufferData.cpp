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

using namespace PicoMqtt;

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
    data = (char *)malloc(length);
    memcpy(data, string, length);
    this->length = length;
}

BufferData::BufferData(const BufferData &source)
{
    state = source.state;
    if (source.data)
    {
        data = (char *)malloc(source.length);
        memcpy(data, source.data, source.length);
    }
    else
    {
        data = NULL;
    }

    length = source.length;
}

BufferData &BufferData::operator=(const BufferData &right)
{
    if (&right != this)
    {
        if (data)
        {
            free(data);
        }

        state = right.state;

        if (right.data)
        {
            length = right.length;
            data = (char *)malloc(right.length);
            memcpy(data, right.data, length);
        }
        else
        {
            length = 0;
            data = NULL;
        }
    }
    return *this;
}

bool BufferData::operator==(const BufferData &right)
{
    return (length == right.length && memcmp(data, right.data, length));
}

bool BufferData::operator!=(const BufferData &right)
{
    return !(*this == right);
}

BufferData::~BufferData()
{
    if (data)
    {
        free(data);
        data = NULL;
    }
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
        if (data != NULL)
        {
            free(data);
            data = NULL;
        }
    }

    if (state == LENGTH && (size_t)client->available() >= STRING_LENGTH_SIZE)
    {
        // read += client->read(&length, STRING_LENGTH_SIZE);
        length.readFromClient(client, read);
        state = DATA;
        if (length > 0)
        {
            data = (char *)malloc(length);
            memset(data, 0, length);
        }
    }

    if (state == DATA && (size_t)client->available() >= length)
    {
        read += client->read(data, length);
        state = IDLE;
        return false;
    }

    return true;
}