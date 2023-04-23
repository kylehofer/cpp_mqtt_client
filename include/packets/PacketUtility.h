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

/**
 * @brief Utility file for handling Packets
 *
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
#include "Client.h"

namespace PicoMqtt
{
    /**
     * @brief Attempts to read a packet from the client.
     * Keeps a progressively running buffer of data until the entire packet is read and
     * the packet can be processed
     *
     * @return Packet* The processed packet, NULL if a complete packet has not been receieved.
     * Packet destruction must be handled by the caller
     */
    Packet *readPacketFromClient(Client *);

    // /**
    //  * @brief Constructs a packet from an identifier
    //  *
    //  * @param identifier
    //  * @return Packet*
    //  */
    // static Packet *constructPacketFromId(uint8_t identifier);
}

#endif /* PACKETUTILITY */
