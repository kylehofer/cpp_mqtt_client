/*
 * File: Payload.cpp
 * Project: cpp_mqtt_client
 * Created Date: Wednesday March 15th 2023
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

#include "types/Payload.h"
#include <algorithm>

using namespace PicoMqtt;

Payload::Payload()
{
}

Payload::Payload(Payload &payload)
{
    if (payload.data)
    {
        length = payload.length;
        data = (uint8_t *)malloc(payload.length);
        memcpy(data, payload.data, length);
    }
    else
    {
        length = 0;
        data = NULL;
    }
}

Payload::Payload(uint32_t length)
{
    this->length = length;
    data = (uint8_t *)malloc(length);
}

Payload::Payload(void *data, uint32_t length) : Payload(length)
{
    memcpy(this->data, data, length);
}

Payload::~Payload()
{
    if (data)
    {
        free(data);
    }
}

Payload &Payload::operator=(const Payload &right)
{
    if (&right != this)
    {
        if (data)
        {
            free(data);
        }

        if (right.data)
        {
            length = right.length;
            data = (uint8_t *)malloc(right.length);
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

uint8_t *Payload::getData()
{
    return data;
}

size_t Payload::size()
{
    return length;
}

size_t Payload::pushToClient(Client *client)
{
    return client->write(data, length);
}

bool Payload::readFromClient(Client *client, uint32_t &bytes)
{
    while (client->available() > 0 && bytesRead < length)
    {
        uint32_t toRead = std::min((length - bytesRead), (uint32_t)client->available());
        client->read(data + bytesRead, toRead);

        bytesRead += toRead;
        bytes += toRead;
    }
    return bytesRead < length;
}
