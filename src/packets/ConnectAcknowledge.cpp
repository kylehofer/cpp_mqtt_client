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

ConnectAcknowledge::ConnectAcknowledge() : PropertiesPacket(0)
{
}

ConnectAcknowledge::ConnectAcknowledge(uint8_t flags) : PropertiesPacket(PacketId::CONNECT_ACKNOWLEDGE | (flags & HEADER_BYTES_MASK))
{
    header.data = 0;
}

bool ConnectAcknowledge::readFromClient(Client *client, uint32_t &read)
{
    uint32_t start = getRemainingLength();

    if (state == IDLE || state == COMPLETE)
    {
        state = VARIABLE_HEADER_FIXED;
        properties.clear();
    }

    while (client->available() > 0 && dataRemaining() && state != COMPLETE)
    {
        uint32_t bytes = 0;
        switch (state)
        {
        case VARIABLE_HEADER_FIXED:
            if (client->available() > VARIABLE_HEADER_FIXED_SIZE)
            {
                bytes += client->read(&(header.data), VARIABLE_HEADER_FIXED_SIZE);
                state = VARIABLE_HEADER_PROPERTIES;
            }
            break;
        case VARIABLE_HEADER_PROPERTIES:
            if (!properties.readFromClient(client, bytes))
            {
                state = COMPLETE;
            }
        default:
            break;
        }

        readBytes(bytes);
    }

    read += getRemainingLength() - start;

    return state != COMPLETE;
}

uint8_t ConnectAcknowledge::getReasonCode()
{
    return header.reasonCode;
}

uint32_t ConnectAcknowledge::getSessionExpiryInterval()
{
    Property *property = properties.get(SESSION_EXPIRY_INTERVAL);

    if (property != NULL)
    {
        return ((SessionExpiryInterval *)property)->getValue();
    }
    return 0;
}

uint16_t ConnectAcknowledge::getReceiveMaximum()
{
    Property *property = properties.get(RECEIVE_MAXIMUM);

    if (property != NULL)
    {
        return ((ReceiveMaxium *)property)->getValue();
    }
    // MQTT 5 specifies if not present to use 16 bit max.
    return 0xFFFF;
}

uint8_t ConnectAcknowledge::getMaximumQoS()
{
    Property *property = properties.get(MAXIMUM_QOS);

    if (property != NULL)
    {
        return ((MaxiumumQoS *)property)->getValue();
    }
    // MQTT 5 specifies if not present to use a max QOS of 2
    return 2;
}

bool ConnectAcknowledge::getRetainAvailable()
{
    Property *property = properties.get(RETAIN_AVAILABLE);

    if (property != NULL)
    {
        return ((RetainAvailable *)property)->getValue();
    }
    return true;
}

uint32_t ConnectAcknowledge::getMaximumPacketSize()
{
    Property *property = properties.get(MAXIMUM_PACKET_SIZE);

    if (property != NULL)
    {
        return ((MaximumPacketSize *)property)->getValue();
    }
    // MQTT 5 specifies if not present to allow packet size upto what is supported
    // by the communication protocol
    return 0;
}

EncodedString ConnectAcknowledge::getAssignedClientIdentifier()
{
    Property *property = properties.get(ASSIGNED_CLIENT_IDENTIFER);

    if (property != NULL)
    {
        return ((AssignedClientIdentifier *)property)->getValue();
    }
    return EncodedString();
}

uint16_t ConnectAcknowledge::getTopicAliasMaximum()
{
    Property *property = properties.get(TOPIC_ALIAS_MAXIMUM);

    if (property != NULL)
    {
        return ((TopicAliasMaximum *)property)->getValue();
    }
    // MQTT 5 specifies no value means that Topic Alias are not supported
    return 0;
}

EncodedString ConnectAcknowledge::getReasonString()
{
    Property *property = properties.get(REASON_STRING);

    if (property != NULL)
    {
        return ((ReasonString *)property)->getValue();
    }
    // MQTT 5 specifies no value means that Topic Alias are not supported
    return EncodedString();
}

bool ConnectAcknowledge::getWildcardSubscriptionAvailable()
{
    Property *property = properties.get(WILDCARD_SUBSCRIPTION_AVAILABLE);

    if (property != NULL)
    {
        return ((WildcardSubscriptionAvailable *)property)->getValue();
    }
    // MQTT 5 specifies no value means that Wildcard Subscriptions are Available
    return true;
}

bool ConnectAcknowledge::getSubscriptionIdentifiersAvailable()
{
    Property *property = properties.get(SUBSCRIPTION_IDENTIFIERS_AVAILABLE);

    if (property != NULL)
    {
        return ((SubscriptionIdentifierAvailable *)property)->getValue();
    }
    // MQTT 5 specifies no value means that Subscription Identifiers are Available
    return true;
}

bool ConnectAcknowledge::getSharedSubscriptionAvailable()
{
    Property *property = properties.get(SHARED_SUBSCRIPTION_AVAILABLE);

    if (property != NULL)
    {
        return ((SharedSubscriptionAvailable *)property)->getValue();
    }
    // MQTT 5 specifies no value means that Shared Subscriptions are Available
    return true;
}

uint16_t ConnectAcknowledge::getServerKeepAlive()
{
    Property *property = properties.get(SERVER_KEEP_ALIVE);

    if (property != NULL)
    {
        return ((ServerKeepAlive *)property)->getValue();
    }
    // Will treat zero as no value
    return 0;
}

EncodedString ConnectAcknowledge::getResponseInformation()
{
    Property *property = properties.get(RESPONSE_INFORMATION);

    if (property != NULL)
    {
        return ((ResponseInformation *)property)->getValue();
    }
    return EncodedString();
}

EncodedString ConnectAcknowledge::getServerReference()
{
    Property *property = properties.get(SERVER_REFERENCE);

    if (property != NULL)
    {
        return ((ServerReference *)property)->getValue();
    }
    return EncodedString();
}

EncodedString ConnectAcknowledge::getAuthenticationMethod()
{
    Property *property = properties.get(AUTHENTICATION_METHOD);

    if (property != NULL)
    {
        return ((AuthenticationMethod *)property)->getValue();
    }
    return EncodedString();
}

BinaryData ConnectAcknowledge::getAuthenticationData()
{
    Property *property = properties.get(AUTHENTICATION_METHOD);

    if (property != NULL)
    {
        return ((AuthenticationMethod *)property)->getValue();
    }
    return BinaryData();
}

bool ConnectAcknowledge::validate()
{

    return getFixedHeaderFlags() == 0 && // All Fixed flags should be zero
           ((header.data >> 9) == 0);    // Bits 1-7 of the variable header are reserved and must be 0
    // TODO: Reason Code Validation
}
