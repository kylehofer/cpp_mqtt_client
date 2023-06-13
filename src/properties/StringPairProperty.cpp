/*
 * File: StringPairProperty.cpp
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
#include <string.h>

using namespace PicoMqtt;

enum ReadingState
{
    KEY,
    VALUE,
    IDLE
};

StringPairProperty::StringPairProperty()
{
    state = IDLE;
}

StringPairProperty::StringPairProperty(PropertyCodes identifier)
    : Property(identifier)
{
    state = IDLE;
}

StringPairProperty::StringPairProperty(PropertyCodes identifier, EncodedString key, EncodedString value)
    : Property(identifier), key(key), value(value)
{
    state = IDLE;
}

size_t StringPairProperty::propertySize()
{
    return key.size() + value.size();
}

bool StringPairProperty::readFromClient(Client *client, uint32_t &read)
{
    if (state == IDLE)
    {
        state = KEY;
    }

    if (state == KEY && !key.readFromClient(client, read))
    {
        state = VALUE;
    }

    if (state == VALUE && !value.readFromClient(client, read))
    {
        state = IDLE;
        return false;
    }

    return true;
}

size_t StringPairProperty::pushProperty(PacketBuffer &buffer)
{
    return key.push(buffer) + value.push(buffer);
}

void StringPairProperty::setKey(EncodedString key)
{
    this->key = key;
}

EncodedString &StringPairProperty::getKey()
{

    return key;
}

void StringPairProperty::setValue(EncodedString value)
{
    this->value = value;
}

EncodedString &StringPairProperty::getValue()
{
    return value;
}