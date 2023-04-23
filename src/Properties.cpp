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

#include "PicoMqttProperties.h"
#include "stdlib.h"

#define PROPERTY_POINTER_SIZE sizeof(Property *)

using namespace std;

enum PropertiesReadState
{
    LENGTH = 0,
    IDENTIFIER,
    PROPERTY_VALUE,
    COMPLETE
};

using namespace PicoMqtt;

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

size_t Properties::pushToClient(Client *client)
{
    VariableByteInteger propertiesLength;

    propertiesLength.value = size();

    size_t written = propertiesLength.pushToClient(client);

    for (auto &property : properties)
    {
        written += property->pushToClient(client);
    };

    return written;
}

void Properties::addProperty(Property *property)
{
    properties.push_back(property);
}

bool Properties::readFromClient(Client *client, uint32_t *read)
{
    static VariableByteInteger propertiesLength;
    static VariableByteInteger propertyIdentifier;
    static Property *property;
    static uint32_t bytesRead = 0;

    while (client->available() > 0 && (state == PropertiesReadState::LENGTH || bytesRead < propertiesLength.value))
    {
        switch (state)
        {
        case PropertiesReadState::LENGTH:
            while (client->available() > 0)
            {
                if (!propertiesLength.readFromClient(client, &bytesRead))
                {
                    bytesRead = 0;
                    state = PropertiesReadState::IDENTIFIER;
                    break;
                }
            }
            break;
        case PropertiesReadState::IDENTIFIER:
            while (client->available() > 0)
            {
                if (!propertyIdentifier.readFromClient(client, &bytesRead))
                {
                    state = PropertiesReadState::PROPERTY_VALUE;
                    property = constructPropertyFromId(static_cast<PropertyCodes>(propertyIdentifier.value));
                    break;
                }
            }
            break;
        case PropertiesReadState::PROPERTY_VALUE:
            if (!property->readFromClient(client, &bytesRead))
            {
                addProperty(property);
                propertyIdentifier.value = 0;
                if (bytesRead < propertiesLength.value)
                {
                    state = PropertiesReadState::IDENTIFIER;
                }
            }
            break;
        default:
            break;
        }
    }

    read += bytesRead;

    if (bytesRead >= propertiesLength.value)
    {

        state = PropertiesReadState::LENGTH;
        bytesRead = 0;
        propertiesLength.value = 0;
        return false;
    }
    return true;
}

Property *Properties::constructPropertyFromId(PropertyCodes identifier)
{
    switch (identifier)
    {
    case PAYLOAD_FORMAT_INDICATOR:
        return (Property *)new PayloadFormatIndicatorProperty();
    case MESSAGE_EXPIRY_INTERVAL:
        return (Property *)new MessageExpiryIntervalProperty();
    case CONTENT_TYPE:
        return (Property *)new ContentTypeProperty();
    case RESPONSE_TOPIC:
        return (Property *)new ResponseTopic();
    case CORRELATION_DATA:
        return (Property *)new CorrelationData();
    case SUBSCRIPTION_IDENTIFIER:
        return (Property *)new SubscriptionIdentifier();
    case SESSION_EXPIRY_INTERVAL:
        return (Property *)new SessionExpiryInterval();
    case ASSIGNED_CLIENT_IDENTIFER:
        return (Property *)new AssignedClientIdentifier();
    case SERVER_KEEP_ALIVE:
        return (Property *)new ServerKeepAlive();
    case AUTHENTICATION_METHOD:
        return (Property *)new AuthenticationMethod();
    case AUTHENTICATION_DATA:
        return (Property *)new AuthenticationData();
    case REQUEST_PROBLEM_INFORMATION:
        return (Property *)new RequestProblemInformation();
    case WILL_DELAY_INTERVAL:
        return (Property *)new WillDelayInterval();
    case REQUEST_RESPONSE_INFORMATION:
        return (Property *)new RequestResponseInformation();
    case RESPONSE_INFORMATION:
        return (Property *)new ResponseInformation();
    case SERVER_REFERENCE:
        return (Property *)new ServerReference();
    case REASON_STRING:
        return (Property *)new ReasonString();
    case RECEIVE_MAXIMUM:
        return (Property *)new ReceiveMaxium();
    case TOPIC_ALIAS_MAXIMUM:
        return (Property *)new TopicAliasMaximum();
    case TOPIC_ALIAS:
        return (Property *)new TopicAlias();
    case MAXIMUM_QOS:
        return (Property *)new MaxiumumQoS();
    case RETAIN_AVAILABLE:
        return (Property *)new RetainAvailable();
    case USER_PROPERTY:
        return (Property *)new UserProperty();
    case MAXIMUM_PACKET_SIZE:
        return (Property *)new MaximumPacketSize();
    case WILDCARD_SUBSCRIPTION_AVAILABLE:
        return (Property *)new WildcardSubscriptionAvailable();
    case SUBSCRIPTION_IDENTIFIERS_AVAILABLE:
        return (Property *)new SubscriptionIdentifierAvailable();
    case SHARED_SUBSCRIPTION_AVAILABLE:
        return (Property *)new SharedSubscriptionAvailable();
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
    for (auto &property : properties)
    {
        delete property;
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
    auto result = ranges::find_if(properties.begin(), properties.end(),
                                  [identifier](Property *property)
                                  { return property->getIdentifier() == identifier; });

    return (result != properties.end()) ? *result : NULL;
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

    auto result = properties.begin();
    auto end = properties.end();

    while (end != (result = ranges::find_if(result, end, filter)))
    {
        callback(*result);
    };
}

// for (auto property : properties)
// {
//     if (property->getIdentifier() == identifier)
//     {
//         return property;
//     }
// }
// return NULL;