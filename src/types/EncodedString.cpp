/*
 * File: EncodedString.cpp
 * Project: cpp_mqtt_client
 * Created Date: Tuesday March 7th 2023
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

#include "types/EncodedString.h"

using namespace PicoMqtt;

#define STRING_LENGTH_SIZE sizeof(uint32_t)

enum ReadingState
{
    IDLE = 0,
    LENGTH,
    DATA
};

EncodedString::EncodedString()
{
    state = IDLE;
}

EncodedString::EncodedString(const char *string, uint32_t length)
{
    state = IDLE;
    data = (char *)malloc(length);
    memcpy(data, string, length);
    this->length = length;
}

EncodedString::EncodedString(const EncodedString &source)
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

EncodedString &EncodedString::operator=(const EncodedString &right)
{
    if (&right != this)
    {
        EncodedString temp(right);
        // std::swap(data, tmp.data);
        char *tempData = data;
        uint32_t tempLength = length;
        data = temp.data;
        length = temp.length;
        temp.data = tempData;
        temp.length = tempLength;
    }
    return *this;
}

EncodedString::~EncodedString()
{
    if (data)
    {
        free(data);
        data = NULL;
    }
}

size_t EncodedString::pushToClient(Client *client)
{
    size_t lengthLength = client->write(&length, STRING_LENGTH_SIZE);
    size_t valueLength = client->write(data, length);

    return size();
}

size_t EncodedString::pushToBuffer(void *buffer)
{
    char *charBuffer = (char *)buffer;
    memcpy(charBuffer, &length, STRING_LENGTH_SIZE);
    charBuffer += STRING_LENGTH_SIZE;
    memcpy(charBuffer, data, length);
    return size();
};

size_t EncodedString::size()
{
    return length + STRING_LENGTH_SIZE;
}

bool EncodedString::readFromClient(Client *client, uint32_t *read)
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

    if (state == LENGTH && client->available() >= STRING_LENGTH_SIZE)
    {
        *read += client->read(&length, STRING_LENGTH_SIZE);
        state = DATA;
        if (length > 0)
        {
            data = (char *)malloc(length);
            memset(data, 0, length);
        }
    }

    if (state == DATA && client->available() >= length)
    {
        *read += client->read(data, length);
        state = IDLE;
        return false;
    }

    return true;
}