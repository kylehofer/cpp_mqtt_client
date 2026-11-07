/*
 * File: Properties.cpp
 * Project: cpp_mqtt_client
 * Created Date: Sunday February 26th 2023
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

#include "MqttProperties.h"
#include "stdlib.h"

#define PROPERTY_POINTER_SIZE sizeof(Property *)

using namespace std;

enum PropertiesReadState
{
    LENGTH = 0,
    IDENTIFIER,
    PROPERTY_VALUE,
    SKIP_REMAINING,
    COMPLETE
};

using namespace CppMqtt;

uint32_t Properties::length()
{
    return properties.size();
}

size_t Properties::totalSize()
{
    VariableByteInteger propertiesLength;

    propertiesLength = size();

    return propertiesLength.size() + propertiesLength.value;
}

size_t Properties::size()
{
    size_t size = 0;

    for (auto &property : properties)
    {
        size += property->size();
    };

    return size;
}

size_t Properties::push(PacketBuffer &buffer)
{
    VariableByteInteger propertiesLength;

    propertiesLength.value = size();

    size_t written = propertiesLength.push(buffer);

    for (auto &property : properties)
    {
        written += property->push(buffer);
    };

    return written;
}

bool Properties::addProperty(Property *property)
{
    if (property == NULL)
    {
        return false;
    }
    if (!pushBack(properties, property))
    {
        destroyProperty(property);
        return false;
    }
    return true;
}

bool Properties::readFromClient(Client *client, uint32_t &read)
{
    while (client->available() > 0 && (state == PropertiesReadState::LENGTH || readCount < readLength.value))
    {
        const uint32_t progress = readCount + read;

        switch (state)
        {
        case PropertiesReadState::LENGTH:
            if (!readLength.readFromClient(client, read))
            {
                readCount = 0;
                state = PropertiesReadState::IDENTIFIER;
                // break;b
            }
            break;
        case PropertiesReadState::IDENTIFIER:
            while (client->available() > 0)
            {
                if (!readIdentifier.readFromClient(client, readCount))
                {

                    readProperty = constructPropertyFromId(static_cast<PropertyCodes>(readIdentifier.value));
                    readIdentifier.value = 0;
                    // An unknown identifier means the value length is unknown, so the
                    // rest of the properties block is skipped instead of dereferencing NULL
                    state = (readProperty != NULL) ? PropertiesReadState::PROPERTY_VALUE : PropertiesReadState::SKIP_REMAINING;
                    break;
                }
            }
            break;
        case PropertiesReadState::PROPERTY_VALUE:
            if (!readProperty->readFromClient(client, readCount))
            {
                addProperty(readProperty);
                readProperty = NULL;
                if (readCount < readLength.value)
                {
                    state = PropertiesReadState::IDENTIFIER;
                }
            }
            break;
        case PropertiesReadState::SKIP_REMAINING:
        {
            uint8_t discard[16];
            uint32_t toRead = readLength.value - readCount;
            if (toRead > sizeof(discard))
            {
                toRead = sizeof(discard);
            }
            if ((uint32_t)client->available() < toRead)
            {
                toRead = client->available();
            }
            readCount += client->read(discard, toRead);
        }
        break;
        default:
            break;
        }

        // No progress: wait for more data rather than spinning on a partial property
        if (readCount + read == progress)
        {
            break;
        }
    }

    if (readCount >= readLength.value)
    {
        read += readCount;
        state = PropertiesReadState::LENGTH;
        readCount = 0;
        readLength.value = 0;
        return false;
    }
    return true;
}

Property *Properties::constructPropertyFromId(PropertyCodes identifier)
{
    switch (identifier)
    {
    case PAYLOAD_FORMAT_INDICATOR:
        return createProperty<PayloadFormatIndicatorProperty>();
    case MESSAGE_EXPIRY_INTERVAL:
        return createProperty<MessageExpiryIntervalProperty>();
    case CONTENT_TYPE:
        return createProperty<ContentTypeProperty>();
    case RESPONSE_TOPIC:
        return createProperty<ResponseTopic>();
    case CORRELATION_DATA:
        return createProperty<CorrelationData>();
    case SUBSCRIPTION_IDENTIFIER:
        return createProperty<SubscriptionIdentifier>();
    case SESSION_EXPIRY_INTERVAL:
        return createProperty<SessionExpiryInterval>();
    case ASSIGNED_CLIENT_IDENTIFER:
        return createProperty<AssignedClientIdentifier>();
    case SERVER_KEEP_ALIVE:
        return createProperty<ServerKeepAlive>();
    case AUTHENTICATION_METHOD:
        return createProperty<AuthenticationMethod>();
    case AUTHENTICATION_DATA:
        return createProperty<AuthenticationData>();
    case REQUEST_PROBLEM_INFORMATION:
        return createProperty<RequestProblemInformation>();
    case WILL_DELAY_INTERVAL:
        return createProperty<WillDelayInterval>();
    case REQUEST_RESPONSE_INFORMATION:
        return createProperty<RequestResponseInformation>();
    case RESPONSE_INFORMATION:
        return createProperty<ResponseInformation>();
    case SERVER_REFERENCE:
        return createProperty<ServerReference>();
    case REASON_STRING:
        return createProperty<ReasonString>();
    case RECEIVE_MAXIMUM:
        return createProperty<ReceiveMaxium>();
    case TOPIC_ALIAS_MAXIMUM:
        return createProperty<TopicAliasMaximum>();
    case TOPIC_ALIAS:
        return createProperty<TopicAlias>();
    case MAXIMUM_QOS:
        return createProperty<MaxiumumQoS>();
    case RETAIN_AVAILABLE:
        return createProperty<RetainAvailable>();
    case USER_PROPERTY:
        return createProperty<UserProperty>();
    case MAXIMUM_PACKET_SIZE:
        return createProperty<MaximumPacketSize>();
    case WILDCARD_SUBSCRIPTION_AVAILABLE:
        return createProperty<WildcardSubscriptionAvailable>();
    case SUBSCRIPTION_IDENTIFIERS_AVAILABLE:
        return createProperty<SubscriptionIdentifierAvailable>();
    case SHARED_SUBSCRIPTION_AVAILABLE:
        return createProperty<SharedSubscriptionAvailable>();
    default:
        break;
    }
    return NULL;
}

Properties::Properties()
{
}

Properties::Properties(__attribute__((unused)) uint32_t count)
{
}

void Properties::clear()
{
    // Discard any partially read property and reset the parse state
    destroyProperty(readProperty);
    readProperty = NULL;
    readLength.value = 0;
    readIdentifier.value = 0;
    readCount = 0;
    state = 0;

    for (auto &property : properties)
    {
        destroyProperty(property);
    };

    properties.clear();
}

Properties::~Properties()
{
    clear();
}

bool Properties::has(PropertyCodes identifier)
{
    return (get(identifier) != NULL);
}

Property *Properties::get(PropertyCodes identifier)
{
    for (auto &property : properties)
    {
        if (property->getIdentifier() == identifier)
        {
            return property;
        }
    }
    return NULL;
}

// // void Properties::each(PropertyCodes identifier, void (*callback)(T *))
// template <typename T>
// void Properties::each(PropertyCodes identifier, std::function<void(T *)> const &lambda)
// {

//     auto result = properties.begin();
//     auto end = properties.end();

//     while (end != (result = std::ranges::find_if(result, end,
//                                                  [identifier](Property *property)
//                                                  { return property->getIdentifier() == identifier; })))
//     {
//         callback((T *)*result);
//     };
// }
void Properties::each(bool (*filter)(Property *), void (*callback)(Property *))
{
    for (auto &property : properties)
    {
        if (filter(property))
        {
            callback(property);
        }
    }
}

// for (auto property : properties)
// {
//     if (property->getIdentifier() == identifier)
//     {
//         return property;
//     }
// }
// return NULL;