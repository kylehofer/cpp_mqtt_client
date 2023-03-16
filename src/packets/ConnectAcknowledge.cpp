/*
 * File: ConnectAcknowledge.cpp
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

#include "packets/ConnectAcknowledge.h"

using namespace PicoMqtt;

enum ReadingState
{
    IDLE = 0,
    VARIABLE_HEADER_FIXED,
    VARIABLE_HEADER_PROPERTIES,
    COMPLETE
};

#define VARIABLE_HEADER_FIXED_SIZE 2

ConnectAcknowledge::ConnectAcknowledge() : Packet(CONNECT_ACKNOWLEDGE_ID)
{
}

bool ConnectAcknowledge::readFromClient(Client *client, uint32_t *bytes)
{
    uint32_t start = getRemainingLength();

    if (state == IDLE || state == COMPLETE)
    {
        state = VARIABLE_HEADER_FIXED;
        properties.clear();
    }

    while (dataRemaining() && state != COMPLETE)
    {
        uint32_t bytes = 0;
        switch (state)
        {
        case VARIABLE_HEADER_FIXED:
            if (client->available() > VARIABLE_HEADER_FIXED_SIZE)
            {
                bytes += client->read(&flags.data, VARIABLE_HEADER_FIXED_SIZE);
                state = VARIABLE_HEADER_PROPERTIES;
            }
            break;
        case VARIABLE_HEADER_PROPERTIES:
            if (!properties.readFromClient(client, &bytes))
            {
                state = COMPLETE;
            }
        default:
            break;
        }

        readBytes(bytes);
    }

    *bytes += getRemainingLength() - start;

    return state == COMPLETE;
}
