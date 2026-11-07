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

using namespace CppMqtt;

Payload::Payload()
{
}

#ifdef STATIC_MEMORY
/*
 * With STATIC_MEMORY a Payload never owns memory. Constructing from data,
 * copying and assigning all refer to the same bytes, which must stay valid for
 * as long as the Payload is used.
 */
Payload::Payload(const Payload &payload)
    : data(payload.data), length(payload.length), ownership(false), truncated(payload.truncated)
{
}

Payload::Payload(void *data, uint32_t length) : data((uint8_t *)data), length(length), ownership(false)
{
}

Payload &Payload::operator=(const Payload &right)
{
    if (&right != this)
    {
        data = right.data;
        length = right.length;
        ownership = false;
        truncated = right.truncated;
        bytesRead = 0;
    }
    return *this;
}

void Payload::setData(void *data, uint32_t length)
{
    setReference(data, length);
}
#else
Payload::Payload(const Payload &payload) : truncated(payload.truncated)
{
    if (payload.data && payload.length > 0)
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
    if (length > 0)
    {
        memcpy(this->data, data, length);
    }
}

Payload &Payload::operator=(const Payload &right)
{
    if (&right != this)
    {
        // Memory from wrap() belongs to the caller and must not be freed here
        if (data && ownership)
        {
            free(data);
        }

        // The copy always owns its own buffer and starts a fresh read
        ownership = true;
        bytesRead = 0;
        truncated = right.truncated;

        if (right.data && right.length > 0)
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

void Payload::setData(void *data, uint32_t length)
{
    // Copies the data, like the Payload(void *, uint32_t) constructor
    *this = Payload(data, length);
}
#endif

Payload::~Payload()
{
#ifndef STATIC_MEMORY
    if (data && ownership)
    {
        free(data);
    }
#endif
}

Payload Payload::wrap(void *data, uint32_t length)
{
    Payload payload;
    payload.setReference(data, length);
    return payload;
}

#ifndef STATIC_MEMORY
void Payload::allocate(uint32_t length)
{
    if (data && ownership)
    {
        free(data);
    }
    data = (length > 0) ? (uint8_t *)malloc(length) : NULL;
    this->length = (data != NULL) ? length : 0;
    ownership = true;
    truncated = false;
    bytesRead = 0;
}
#endif

void Payload::setReference(void *data, uint32_t length)
{
#ifndef STATIC_MEMORY
    if (this->data && ownership)
    {
        free(this->data);
    }
#endif
    this->data = (uint8_t *)data;
    this->length = length;
    ownership = false;
    truncated = false;
    bytesRead = 0;
}

uint8_t *Payload::getData()
{
    return data;
}

size_t Payload::size()
{
    return length;
}

bool Payload::wasTruncated()
{
    return truncated;
}

void Payload::setTruncated(bool value)
{
    truncated = value;
}

size_t Payload::push(PacketBuffer &buffer)
{
    return buffer.push(data, length);
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
