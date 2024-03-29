/*
 * File: VariableByteInteger.cpp
 * Project: cpp_mqtt_client
 * Created Date: Tuesday March 7th 2023
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "types/VariableByteInteger.h"

using namespace CppMqtt;

#define LOWER_SEVEN_BITS 0x7F

VariableByteInteger::VariableByteInteger()
{
}

VariableByteInteger::VariableByteInteger(uint32_t value)
{
    this->value = value;
}

size_t VariableByteInteger::size()
{
    if (value < 128)
    {
        return 1;
    }
    if (value < 16384)
    {
        return 2;
    }
    if (value < 2097152)
    {
        return 3;
    }

    return 4;
};

size_t VariableByteInteger::push(PacketBuffer &buffer)
{
    size_t length = size();
    uint32_t value = this->value;
    uint8_t output[4];

    for (size_t i = 0; i < length; i++)
    {
        if (i > 0)
        {
            output[i - 1] |= 0x80;
        }
        output[i] = 0x7F & value;
        value >>= 7;
    }

    buffer.push(output, length);
    return length;
};

uint32_t VariableByteInteger::calculateValue(uint32_t input)
{
    return (((input & 0x800000) > 0) * (input & 0x7F000000)) >> 3 |
           (((input & 0x8000) > 0) * (input & 0x7F0000)) >> 2 |
           (((input & 0x80) > 0) * (input & 0x7F00)) >> 1 |
           (input & 0x7F);
}

bool VariableByteInteger::addByte(uint8_t byte)
{
    if ((value & 0xFF) == 0)
    {
        value = byte;
    }
    else if ((value & 0xFF00) == 0)
    {
        value |= (byte << 8);
    }
    else if ((value & 0xFF0000) == 0)
    {
        value |= byte << 16;
    }
    else
    {
        value |= byte << 24;
        value = calculateValue(value);
        return false;
    }

    if (byte <= 127)
    {
        value = calculateValue(value);
        return false;
    }

    return true;
}

bool VariableByteInteger::readFromClient(Client *client, uint32_t &read)
{
    while (client->available() > 0)
    {
        uint8_t byte;
        read += client->read(&byte, 1);
        if (!addByte(byte))
        {
            return false;
        }
    }

    return true;
}

VariableByteInteger &VariableByteInteger::operator=(const uint32_t right)
{
    this->value = right;
    return *this;
}
