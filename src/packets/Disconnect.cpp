/*
 * File: Disconnect.cpp
 * Project: cpp_mqtt_client
 * Created Date: Thursday March 16th 2023
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

#include "packets/Disconnect.h"

using namespace PicoMqtt;

enum ReadingState
{
    IDLE = 0,
    REASON_CODE,
    PROPERTIES,
    COMPLETE
};

#define REASON_CODE_SIZE 1

Disconnect::Disconnect() : PropertiesPacket(DISCONNECT_ID)
{
}

Disconnect::Disconnect(uint8_t flags) : PropertiesPacket(DISCONNECT_ID | (flags & HEADER_BYTES_MASK))
{
}

size_t Disconnect::push(PacketBuffer &buffer)
{
    // Fixed Header
    size_t written = Packet::push(buffer);

    written += buffer.push(reasonCode);
    written += properties.push(buffer);

    return written;
}

bool Disconnect::readFromClient(Client *client, uint32_t &bytes)
{
    uint32_t start = getRemainingLength();

    if (state == IDLE || state == COMPLETE)
    {
        state = REASON_CODE;
        properties.clear();
        reasonCode = 0;
    }

    while (client->available() > 0 && dataRemaining() && state != COMPLETE)
    {
        uint32_t read = 0;
        switch (state)
        {
        case REASON_CODE:
            if (client->available() >= REASON_CODE_SIZE)
            {
                read += client->read(&reasonCode, REASON_CODE_SIZE);
                state = REASON_CODE;
            }
            break;
        case PROPERTIES:
            if (!properties.readFromClient(client, read))
            {
                state = COMPLETE;
            }
            break;
        default:
            break;
        }

        readBytes(read);
    }

    bytes += getRemainingLength() - start;
    return state == COMPLETE;
}

size_t Disconnect::size()
{
    return REASON_CODE_SIZE + properties.size();
}

void Disconnect::setReasonCode(uint8_t value)
{
    reasonCode = value;
}

uint8_t Disconnect::getReasonCode()
{
    return reasonCode;
}

bool Disconnect::validate()
{
    return true;
}
