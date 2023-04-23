/*
 * File: ByteProperty.cpp
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

using namespace PicoMqtt;

ByteProperty::ByteProperty()
{
}

ByteProperty::ByteProperty(PropertyCodes identifier) : Property(identifier), value(0)
{
}

ByteProperty::ByteProperty(PropertyCodes identifier, uint8_t value) : Property(identifier), value(value)
{
}

size_t ByteProperty::propertySize()
{
    return sizeof(uint8_t);
}

bool ByteProperty::readFromClient(Client *client, uint32_t *read)
{
    if (client->available() < 1)
    {
        return true;
    }

    *read += client->read(&value, 1);
    return false;
}

size_t ByteProperty::pushPropertyToClient(Client *client)
{
    return client->write(value);
}

void ByteProperty::setValue(uint8_t value)
{
    this->value = value;
}

uint8_t ByteProperty::getValue()
{
    return value;
}