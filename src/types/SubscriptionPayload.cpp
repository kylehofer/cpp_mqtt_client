/*
 * File: SubscriptionPayload.cpp
 * Project: cpp_mqtt_client
 * Created Date: Thursday March 16th 2023
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

#include "types/SubscriptionPayload.h"

using namespace CppMqtt;

size_t SubscriptionPayload::push(PacketBuffer &buffer)
{
    return topic.push(buffer);
}

bool SubscriptionPayload::readFromClient([[maybe_unused]] Client *client, [[maybe_unused]] uint32_t &read)
{
    // Not Used
    return false;
}

size_t SubscriptionPayload::size()
{
    return topic.size();
}

void SubscriptionPayload::setTopic(EncodedString &topic)
{
    this->topic = EncodedString(topic);
}

void SubscriptionPayload::setTopic(const char *data, uint16_t length)
{
    this->topic = EncodedString(data, length);
}