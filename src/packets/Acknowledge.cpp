/*
 * File: Acknowledge.cpp
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

#include "packets/Acknowledge.h"

using namespace PicoMqtt;

enum ReadingState
{
    IDLE = 0,
    IDENTIFIER,
    REASON_CODE,
    PROPERTIES,
    COMPLETE
};

#define PACKET_IDENTIFIER_SIZE 2
#define REASON_CODE_SIZE 1

size_t Acknowledge::size()
{
    if (properties.length() > 0)
    {
        return PACKET_IDENTIFIER_SIZE + REASON_CODE_SIZE + properties.totalSize();
    }
    if (reasonCode > 0)
    {
        return PACKET_IDENTIFIER_SIZE + REASON_CODE_SIZE;
    }
    return PACKET_IDENTIFIER_SIZE;
}

size_t Acknowledge::push(PacketBuffer &buffer)
{
    size_t bytes = size();
    // Fixed Header
    size_t written = Packet::push(buffer);

    written += buffer.push(&packetIdentifier, PACKET_IDENTIFIER_SIZE);

    if (bytes > (PACKET_IDENTIFIER_SIZE + REASON_CODE_SIZE))
    {
        written += buffer.push(reasonCode);
        written += properties.push(buffer);
    }
    else if (bytes > PACKET_IDENTIFIER_SIZE)
    {
        written += buffer.push(reasonCode);
    }
    return written;
}

bool Acknowledge::readFromClient(Client *client, uint32_t &bytes)
{
    uint32_t start = getRemainingLength();

    if (state == IDLE || state == COMPLETE)
    {
        state = IDENTIFIER;
        properties.clear();
        reasonCode = 0;
    }

    while (client->available() > 0 && dataRemaining() && state != COMPLETE)
    {
        uint32_t read = 0;
        switch (state)
        {
        case IDENTIFIER:
            if (client->available() >= PACKET_IDENTIFIER_SIZE)
            {
                read += client->read(&packetIdentifier, PACKET_IDENTIFIER_SIZE);
                state = REASON_CODE;
            }
            break;
        case REASON_CODE:
            if (client->available() >= REASON_CODE_SIZE)
            {
                read += client->read(&reasonCode, REASON_CODE_SIZE);
                state = PROPERTIES;
            }
            break;
        case PROPERTIES:
            if (!properties.readFromClient(client, read))
            {
                state = COMPLETE;
            }
        default:
            break;
        }

        readBytes(read);
    }

    bytes += getRemainingLength() - start;

    if (!dataRemaining())
    {
        state = COMPLETE;
    }

    return state != COMPLETE;
}

uint8_t Acknowledge::getReasonCode()
{
    return reasonCode;
}

uint16_t Acknowledge::getPacketIdentifier()
{
    return packetIdentifier;
}

void Acknowledge::setReasonCode(uint16_t value)
{
    reasonCode = value;
}

void Acknowledge::setPacketIdentifier(uint16_t value)
{
    packetIdentifier = value;
}