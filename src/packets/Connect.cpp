/*
 * File: Connect.cpp
 * Project: cpp_mqtt_client
 * Created Date: Wednesday March 1st 2023
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

#include "packets/Connect.h"

#define VARIABLE_HEADER_SIZE 10

using namespace PicoMqtt;

Connect::Connect() : Packet(CONNECT_ID)
{
}

size_t Connect::size()
{
    return 0;
}

size_t Connect::pushToClient(Client *client)
{
    // Fixed Header
    size_t written = Packet::pushToClient(client);

    // Variable Header
    written += client->write(PROTOCOL_NAME, PROTOCOL_NAME_LENGTH);
    written += client->write(connectFlags.data);
    written += properties.pushToClient(client);

    // Payload
    written += clientId.pushToClient(client);

    if (connectFlags.will)
    {
        written += willProperties->pushToClient(client);
        written += willTopic->pushToClient(client);
        written += willPayload->pushToClient(client);
    }

    if (connectFlags.username)
        written += userName->pushToClient(client);

    if (connectFlags.password)
        written += password->pushToClient(client);

    return written;
}