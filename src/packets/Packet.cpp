/*
 * File: Packet.cpp
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

#include "packets/Packet.h"
#include <cstdio>

using namespace CppMqtt;

Packet::Packet(FixedHeader fixedHeader) : fixedHeader(fixedHeader)
{
}

Packet::Packet(uint8_t fixedHeader)
{
    this->fixedHeader.data = fixedHeader;
}

Packet::~Packet()
{
}

uint8_t Packet::getFixedHeaderFlags()
{
    return fixedHeader.data & 0xF;
}

uint32_t Packet::getRemainingLength()
{
    return remainingLength < bytesRead ? 0 : remainingLength - bytesRead;
}
void Packet::readBytes(uint32_t count)
{
    bytesRead += count;
}

bool Packet::dataRemaining()
{
    return getRemainingLength() > 0;
}

bool Packet::isValid()
{
    return bytesRead == getRemainingLength();
}

size_t Packet::totalSize()
{
    remainingLength = size();
    return remainingLength.size() + 1 + remainingLength;
}

size_t Packet::push(PacketBuffer &buffer)
{
    buffer.push(fixedHeader.data);
    remainingLength = size();
    return remainingLength.push(buffer) + 1;
}

void Packet::setFlags(uint8_t flags)
{
    uint8_t packetType = getPacketType();
    fixedHeader.data = (flags & 0xF) | packetType;
}

void Packet::setRemainingLength(VariableByteInteger remainingLength)
{
    this->remainingLength = remainingLength;
}

void Packet::setRemainingLength(uint32_t remainingLength)
{
    this->remainingLength = remainingLength;
}

uint8_t Packet::getPacketType()
{
    return (fixedHeader.data & 0xF0);
}