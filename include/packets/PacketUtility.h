/*
 * File: PacketUtility.h
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

#ifndef PACKETUTILITY
#define PACKETUTILITY

#include "Packet.h"
#include "Authentication.h"
#include "Connect.h"
#include "ConnectAcknowledge.h"
#include "Disconnect.h"
#include "PingRequest.h"
#include "PingResponse.h"
#include "Publish.h"
#include "PublishAcknowledge.h"
#include "PublishComplete.h"
#include "PublishReceived.h"
#include "PublishRelease.h"
#include "Subscribe.h"
#include "SubscribeAcknowledge.h"
#include "Unsubscribe.h"
#include "UnsubscribeAcknowledge.h"

namespace PicoMqtt
{

    Packet *constructPacketFromId(uint8_t identifier)
    {
        switch (identifier & 0xF0) // Strip lower 4 bits
        {
        case CONNECT_ID:
            return (Packet *)new Connect();
        case CONNECT_ACKNOWLEDGE_ID:
            return (Packet *)new ConnectAcknowledge();
        case PUBLISH_ID:
            return (Packet *)new Publish();
        case PUBLISH_ACKNOWLEDGE_ID:
            return (Packet *)new PublishAcknowledge();
        case PUBLISH_RECEIVED_ID:
            return (Packet *)new PublishReceived();
        case PUBLISH_RELEASE_ID:
            return (Packet *)new PublishRelease();
        case PUBLISH_COMPLETE_ID:
            return (Packet *)new PublishComplete();
        case SUBSCRIBE_ID:
            return (Packet *)new Subscribe();
        case SUBSCRIBE_ACKNOWLEDGE_ID:
            return (Packet *)new SubscribeAcknowledge();
        case UNSUBSCRIBE_ID:
            return (Packet *)new Unsubscribe();
        case UNSUBSCRIBE_ACKNOWLEDGE_ID:
            return (Packet *)new UnsubscribeAcknowledge();
        case PING_REQUEST_ID:
            return (Packet *)new PingRequest();
        case PING_RESPONSE_ID:
            return (Packet *)new PingResponse();
        case DISCONNECT_ID:
            return (Packet *)new Disconnect();
        case AUTHENTICATION_ID:
            return (Packet *)new Authentication();
        default:
            break;
        }
        return NULL;
    }
}

#endif /* PACKETUTILITY */
