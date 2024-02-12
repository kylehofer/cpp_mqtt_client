/*
 * File: WillProperties.cpp
 * Project: cpp_mqtt_client
 * Created Date: Saturday December 16th 2023
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

#include "WillProperties.h"

using namespace PicoMqtt;

#define BYTE_DATA 0
#define UTF_8_DATA 1

#define PROPERTY_SETTER(IDENTIFIER, CLASS)           \
    if (!has(IDENTIFIER))                            \
    {                                                \
        addProperty(new CLASS(value));               \
    }                                                \
    else                                             \
    {                                                \
        ((CLASS *)get(IDENTIFIER))->setValue(value); \
    }

WillProperties::WillProperties() : Properties()
{
}

void WillProperties::setPayloadFormatIndicator(uint8_t value)
{
    PROPERTY_SETTER(PAYLOAD_FORMAT_INDICATOR, PayloadFormatIndicatorProperty)
}

void WillProperties::setWillDelayInterval(uint32_t value)
{
    PROPERTY_SETTER(WILL_DELAY_INTERVAL, WillDelayInterval)
}

void WillProperties::setMessageExpiryInterval(uint32_t value)
{
    PROPERTY_SETTER(MESSAGE_EXPIRY_INTERVAL, MessageExpiryIntervalProperty)
}

void WillProperties::setContentType(EncodedString value)
{
    PROPERTY_SETTER(CONTENT_TYPE, ContentTypeProperty)
}

void WillProperties::setResponseTopic(EncodedString value)
{
    PROPERTY_SETTER(RESPONSE_TOPIC, ResponseTopic)
}

void WillProperties::setCorrelationData(EncodedString value)
{
    PROPERTY_SETTER(CORRELATION_DATA, CorrelationData)
}

// TODO: Implement
void WillProperties::addUserProperty(__attribute__((unused)) EncodedString key, __attribute__((unused)) EncodedString value)
{
}

void WillProperties::setWillTopic(EncodedString value)
{
    topic = value;
}

void WillProperties::setWillTopic(const char *data, uint16_t length)
{
    topic = EncodedString(data, length);
}

void WillProperties::setUtf8Payload(EncodedString &payload)
{
    setPayloadFormatIndicator(UTF_8_DATA);
    payload = EncodedString(payload);
}

void WillProperties::setUtf8Payload(const char *data, uint16_t length)
{
    setPayloadFormatIndicator(UTF_8_DATA);
    payload = EncodedString(data, length);
}

void WillProperties::setBinaryPayload(BinaryData &payload)
{
    setPayloadFormatIndicator(BYTE_DATA);
    payload = BinaryData(payload);
}

void WillProperties::setBinaryPayload(const char *data, uint16_t length)
{
    setPayloadFormatIndicator(BYTE_DATA);
    payload = BinaryData(data, length);
}

void WillProperties::setQoS(QoS qos)
{
    this->qos = qos;
}

QoS WillProperties::getQoS()
{
    return qos;
}

void WillProperties::setRetain(bool retain)
{
    this->retain = retain;
}

bool WillProperties::getRetain()
{
    return retain;
}

size_t WillProperties::push(PacketBuffer &buffer)
{
    size_t length = Properties::push(buffer);
    length += topic.push(buffer);
    length += payload.push(buffer);
    return length;
}

size_t WillProperties::size()
{
    return Properties::totalSize() + topic.size() + payload.size();
}
