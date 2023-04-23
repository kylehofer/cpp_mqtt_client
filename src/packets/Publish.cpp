/*
 * File: Publish.cpp
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

#include "packets/Publish.h"

using namespace PicoMqtt;

enum ReadingState
{
    IDLE = 0,
    VARIABLE_HEADER_TOPIC,
    VARIABLE_HEADER_IDENTIFIER,
    VARIABLE_HEADER_PROPERTIES,
    VARIABLE_HEADER_PAYLOAD,
    COMPLETE
};

#define QOS_FLAGS 0x6
#define QOS_1 0x2
#define QOS_2 0x4

#define PACKET_IDENTIFIER_SIZE 2

Publish::Publish() : PropertiesPacket(PUBLISH_ID)
{
}

QoS Publish::getQos()
{
    uint8_t qos = (getFixedHeaderFlags() & QOS_FLAGS);
    switch (qos)
    {
    case QOS_1:
        return QoS::ONE;
    case QOS_2:
        return QoS::TWO;
    default:
        break;
    }

    return QoS::ZERO;
}

bool Publish::readFromClient(Client *client, uint32_t *bytes)
{
    uint32_t start = getRemainingLength();

    if (state == IDLE || state == COMPLETE)
    {
        state = VARIABLE_HEADER_TOPIC;
        properties.clear();
    }

    while (dataRemaining() && state != COMPLETE)
    {
        uint32_t read = 0;
        switch (state)
        {
        case VARIABLE_HEADER_TOPIC:
            if (!topic.readFromClient(client, &read))
            {
                state = (getQos() != QoS::ZERO) ? VARIABLE_HEADER_IDENTIFIER : VARIABLE_HEADER_PROPERTIES;
            }
            break;
        case VARIABLE_HEADER_IDENTIFIER:
            if (client->available() >= PACKET_IDENTIFIER_SIZE)
            {
                read += client->read(&packetIdentifier, PACKET_IDENTIFIER_SIZE);
                state = VARIABLE_HEADER_PROPERTIES;
            }
            break;
        case VARIABLE_HEADER_PROPERTIES:
            if (!properties.readFromClient(client, &read))
            {
                state = VARIABLE_HEADER_PAYLOAD;
            }
            break;
        case VARIABLE_HEADER_PAYLOAD:
            if (!payload.readFromClient(client, &read))
            {
                state = COMPLETE;
            }
            break;
        default:
            break;
        }

        readBytes(read);
    }

    *bytes += getRemainingLength() - start;

    return state == COMPLETE;
}

size_t Publish::size()
{
    size_t bytes = 0;

    bytes += topic.size();

    if (getQos() != QoS::ZERO)
    {
        bytes += PACKET_IDENTIFIER_SIZE;
    }

    bytes += properties.totalSize();

    // Payload
    bytes += payload.size();

    return bytes;
}

size_t Publish::pushToClient(Client *client)
{
    // Fixed Header
    size_t written = Packet::pushToClient(client);

    written += topic.pushToClient(client);

    if (getQos() != QoS::ZERO)
    {
        written += client->write(&packetIdentifier, PACKET_IDENTIFIER_SIZE);
    }

    written += properties.pushToClient(client);

    // Payload
    written += payload.pushToClient(client);

    return written;
}

Payload &Publish::getPayload()
{
    return payload;
}

EncodedString &Publish::getTopic()
{
    return topic;
}

void Publish::setTopic(const char *data, uint32_t length)
{
    topic = EncodedString(data, length);
}

void Publish::setTopic(EncodedString value)
{
    topic = value;
}

void Publish::setPayload(void *data, uint32_t length)
{
    payload = Payload(data, length);
}

void Publish::setPayload(Payload value)
{
    payload = value;
}

uint16_t Publish::getPacketIdentifier()
{
    return packetIdentifier;
}

void Publish::setPacketIdentifier(uint16_t packetIdentifier)
{
    this->packetIdentifier = packetIdentifier;
}

void Publish::setQos(QoS qos)
{

    uint8_t currentFlags = (getFixedHeaderFlags() & ~QOS_FLAGS);

    switch (qos)
    {
    case QoS::ONE:
        currentFlags |= QOS_1;
        break;
    case QoS::TWO:
        currentFlags |= QOS_2;
        break;
    default:
        break;
    }

    setFlags(currentFlags);
}
