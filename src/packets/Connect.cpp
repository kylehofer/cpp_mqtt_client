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
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copiessrc/packets/Connect.cpp
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

Connect::Connect() : PropertiesPacket(CONNECT_ID)
{
    memset(connectFlags.data, 0, CONNECT_FLAGS_SIZE);
}

size_t Connect::size()
{
    size_t size = PROTOCOL_NAME_LENGTH + CONNECT_FLAGS_SIZE + properties.totalSize();

    if (connectFlags.will)
        size += willProperties->size();

    if (connectFlags.username)
        size += userName->size();

    if (connectFlags.password)
        size += password->size();

    return size;
}

size_t Connect::pushToClient(Client *client)
{
    // Fixed Header
    size_t written = Packet::pushToClient(client);

    // Variable Header
    written += client->write(PROTOCOL_NAME, PROTOCOL_NAME_LENGTH);
    written += client->write(connectFlags.data, CONNECT_FLAGS_SIZE);

    written += properties.pushToClient(client);

    // Client Id
    written += clientIdentifier.pushToClient(client);

    if (connectFlags.will)
        written += willProperties->pushToClient(client);

    if (connectFlags.username)
        written += userName->pushToClient(client);

    if (connectFlags.password)
        written += password->pushToClient(client);

    return written;
}

void Connect::setClientId(EncodedString value)
{
    clientIdentifier = value;
}

void Connect::setWill(WillProperties *will)
{

    connectFlags.will = (will != NULL);
    willProperties = will;
}

void Connect::setCleanStart(bool value)
{
    connectFlags.clean = value;
}

void Connect::setKeepAliveInterval(uint16_t value)
{
    connectFlags.keepAliveInterval = value;
}

uint16_t Connect::getKeepAliveInterval()
{
    return connectFlags.keepAliveInterval;
}