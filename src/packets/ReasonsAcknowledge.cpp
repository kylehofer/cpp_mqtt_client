/*
 * File: ReasonsAcknowledge.cpp
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

#include "packets/ReasonsAcknowledge.h"

using namespace PicoMqtt;

#define REASON_CODE_SIZE 1
#define PACKET_IDENTIFIER_SIZE 2

enum ReadingState
{
    IDLE = 0,
    IDENTIFIER,
    PROPERTIES,
    REASON_CODES,
    COMPLETE
};

size_t ReasonsAcknowledge::push([[maybe_unused]] PacketBuffer &buffer)
{
    // Not Required
    return 0;
}

bool ReasonsAcknowledge::readFromClient(Client *client, uint32_t &bytes)
{
    uint32_t start = getRemainingLength();

    if (state == IDLE || state == COMPLETE)
    {
        state = IDENTIFIER;
        properties.clear();
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
                state = PROPERTIES;
            }
            break;
        case PROPERTIES:
            if (!properties.readFromClient(client, read))
            {
                state = REASON_CODES;
            }
            break;
        case REASON_CODES:
        {
            uint8_t reasonCode = 0;
            read += client->read(&reasonCode, REASON_CODE_SIZE);
            reasonCodes.push_back(reasonCode);
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

size_t ReasonsAcknowledge::size()
{
    return PACKET_IDENTIFIER_SIZE + properties.size() + reasonCodes.size();
}

vector<uint8_t> ReasonsAcknowledge::getReasonCodes()
{
    return reasonCodes;
}

uint16_t ReasonsAcknowledge::getPacketIdentifier()
{
    return packetIdentifier;
}