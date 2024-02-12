/*
 * File: SubscribePayload.cpp
 * Project: cpp_mqtt_client
 * Created Date: Saturday June 10th 2023
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

#include "types/SubscribePayload.h"

using namespace PicoMqtt;

#define OPTIONS_SIZE 1

SubscribePayload::SubscribePayload()
{
    options.data = 0;
}

size_t SubscribePayload::push(PacketBuffer &buffer)
{
    return SubscriptionPayload::push(buffer) + buffer.push(options.data);
}

size_t SubscribePayload::size()
{
    return OPTIONS_SIZE + SubscriptionPayload::size();
}

void SubscribePayload::setMaximumQos(QoS qos)
{
    switch (qos)
    {
    case QoS::ZERO:
        options.maximumQos = 0;
        break;
    case QoS::ONE:
        options.maximumQos = 1;
        break;
    case QoS::TWO:
        options.maximumQos = 2;
        break;
    default:
        break;
    }
}

QoS SubscribePayload::getMaximumQos()
{
    switch (options.maximumQos)
    {
    case 0:
        return QoS::ZERO;
        break;
    case 1:
        return QoS::ONE;
        break;
    default:
        return QoS::TWO;
    }
}

void SubscribePayload::setNoLocal(bool value)
{
    options.noLocal = value;
}

bool SubscribePayload::getNoLocal()
{
    return options.noLocal;
}

void SubscribePayload::setRetainAsPublished(bool value)
{
    options.retainAsPublished = value;
}

bool SubscribePayload::getRetainAsPublished()
{
    return options.retainAsPublished;
}

void SubscribePayload::setRetainHandling(RetainHandling retain)
{
    switch (retain)
    {
    case RetainHandling::AT_SUBSCRIBE:
        options.retainHandling = 0;
        break;
    case RetainHandling::AT_SUBSCRIBE_NOT_EXIST:
        options.retainHandling = 1;
        break;
    case RetainHandling::NO_RETAIN:
        options.retainHandling = 2;
        break;
    default:
        break;
    }
}

RetainHandling SubscribePayload::getRetainHandling()
{
    switch (options.retainHandling)
    {
    case 0:
        return RetainHandling::AT_SUBSCRIBE;
        break;
    case 1:
        return RetainHandling::AT_SUBSCRIBE_NOT_EXIST;
        break;
    default:
        return RetainHandling::NO_RETAIN;
    }
}
