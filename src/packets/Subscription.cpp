/*
 * File: Subscription.cpp
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

#include "packets/Subscription.h"

using namespace PicoMqtt;

enum ReadingState
{
    IDLE = 0,
    IDENTIFIER,
    PROPERTIES,
    COMPLETE
};

#define PACKET_IDENTIFIER_SIZE 2

size_t Subscription::size()
{
    size_t bytes = PACKET_IDENTIFIER_SIZE;
    bytes += properties.size();

    for (auto &payload : payloads)
    {
        bytes += payload.size();
    }
    return bytes;
}

size_t Subscription::pushToClient(Client *client)
{
    // Fixed Header
    size_t written = Packet::pushToClient(client);

    written += client->write(&packetIdentifier, PACKET_IDENTIFIER_SIZE);
    written += properties.pushToClient(client);

    for (auto &payload : payloads)
    {
        written += payload.pushToClient(client);
    }

    return written;
}

bool Subscription::readFromClient([[maybe_unused]] Client *client, [[maybe_unused]] uint32_t &bytes)
{
    // Not Used
    return false;
}