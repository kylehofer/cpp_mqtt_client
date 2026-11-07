/*
 * File: MqttClient.cpp
 * Project: cpp_mqtt_client
 * Created Date: Friday December 30th 2022
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

#include "MqttClient.h"
#include "packets/PacketUtility.h"
#include <algorithm>

#if defined(ARDUINO) || defined(ESP8266) || defined(ESP32)
#include <Arduino.h>
#endif

// Build with -DDEBUGGING (or -DDEBUGGING=1) to enable debug output
#if defined(DEBUGGING) && DEBUGGING
#define DEBUG(format, ...)          \
    do                              \
    {                               \
        printf("MqttClient: ");     \
        printf(format, ##__VA_ARGS__); \
    } while (0)
#else
#define DEBUG(out, ...) \
    do                  \
    {                   \
    } while (0)
#endif

using namespace std;

#define SECONDS_TO_MS 1000
// MQTT 5 specifies 1.5 times the Keep Alive as the Keep Alive timeout.
// Kept in integer milliseconds per second to avoid floating point (soft float on Cortex-M0+).
#define KEEP_ALIVE_TIMEOUT_MS_PER_SECOND 1500

namespace CppMqtt
{

    Packet *MqttClient::readNextPacket()
    {
        Packet *packet = reader.read(client);

        if (packet == NULL)
        {
            return NULL;
        }

        serverKeepAliveTimeRemaining = ((uint32_t)getKeepAliveInterval() * KEEP_ALIVE_TIMEOUT_MS_PER_SECOND);

        uint8_t packetType = packet->getPacketType();

        switch (packetType)
        {
        case PacketId::CONNECT:
            break;
        case PacketId::CONNECT_ACKNOWLEDGE:
        {
            ConnectAcknowledge *connectAcknowledge = (ConnectAcknowledge *)packet;
            connectionAcknowledged(connectAcknowledge);
        }
        break;

        case PacketId::PUBLISH:
        {
            Publish *publish = (Publish *)packet;
            onPublish(publish);
        }
        break;
        case PacketId::PUBLISH_ACKNOWLEDGE:
            onPublishAcknowledge((PublishAcknowledge *)packet);
            break;
        case PacketId::PUBLISH_RECEIVED:
            onPublishReceived((PublishReceived *)packet);
            break;
        case PacketId::PUBLISH_RELEASE:
            onPublishRelease((PublishRelease *)packet);
            break;
        case PacketId::PUBLISH_COMPLETE:
            onPublishComplete((PublishComplete *)packet);
            break;
        case PacketId::SUBSCRIBE:
            // TODO: Should never receieve
            break;
        case PacketId::SUBSCRIBE_ACKNOWLEDGE:
        {
            SubscribeAcknowledge *acknowledge = (SubscribeAcknowledge *)packet;
            if (handler)
            {
                handler->onSubscribeResult(acknowledge->getPacketIdentifier(), acknowledge->getReasonCodes());
            }
        }
        break;
        case PacketId::UNSUBSCRIBE:
            // TODO: Should never receieve
            break;
        case PacketId::UNSUBSCRIBE_ACKNOWLEDGE:
        {
            ReasonsAcknowledge *acknowledge = (ReasonsAcknowledge *)packet;
            if (handler)
            {
                handler->onUnsubscribeResult(acknowledge->getPacketIdentifier(), acknowledge->getReasonCodes());
            }
        }
        break;
        case PacketId::PING_REQUEST:
        {
            PingResponse response;
            sendPacket(&response);
            break;
        }
        case PacketId::PING_RESPONSE:
            pingResponse();
            break;
        case PacketId::DISCONNECT:
        {
            Disconnect *disconnect = (Disconnect *)packet;
            setConnectionState(ConnectionState::DISCONNECTED);
            if (ReasonCode::_is_valid(disconnect->getReasonCode()))
            {
                if (handler)
                    handler->onDisconnection(ReasonCode::_from_integral_unchecked(disconnect->getReasonCode()));
            }
        }
        break;
        case PacketId::AUTHENTICATION:
        {
            break;
        }
        default:
        {
            DEBUG("Not Recognized: %d\n", packet->getPacketType());
            break;
        }
        }

        releasePacket(packet);

        return NULL;
    }

    MqttClient::MqttClient()
    {
        applyStaticLimits();
    }

    MqttClient::MqttClient(Client *client)
    {
        this->client = client;
        applyStaticLimits();
    }

    MqttClient::MqttClient(Client *client, TransmitBuffer *transmitBuffer) : transmitBuffer(transmitBuffer)
    {
        this->client = client;
        applyStaticLimits();
    }

    void MqttClient::setTransmitBuffer(TransmitBuffer *transmitBuffer)
    {
        this->transmitBuffer = transmitBuffer;
    }

    MqttClient::~MqttClient()
    {
#ifndef STATIC_MEMORY
        if (this->address)
        {
            free(this->address);
        }
#endif
    }

    void MqttClient::applyStaticLimits()
    {
#ifdef STATIC_MEMORY
        // Receive Maximum: incoming QoS 1/2 messages in flight at once, bounded by
        // the fixed token list. Maximum Packet Size: bounded by the payload buffer,
        // so every received payload fits.
        connectPacket.setReceiveMaximum(CPP_MQTT_MAX_INFLIGHT);
        connectPacket.setMaximumPacketSize(CPP_MQTT_MAX_PAYLOAD_LENGTH);
#endif
    }

    int MqttClient::setWill(WillProperties *will)
    {
        connectPacket.setWill(will);
        return 0;
    }

    int MqttClient::connect(const char *address, int port, uint32_t connectTimeout)
    {
        if (clientState != +ConnectionState::DISCONNECTED)
        {
            return 0;
        }

#ifdef STATIC_MEMORY
        if (strlen(address) > CPP_MQTT_MAX_ADDRESS_LENGTH)
        {
            DEBUG("Address longer than CPP_MQTT_MAX_ADDRESS_LENGTH\n");
            return -1;
        }
        strcpy(this->address, address);
#else
        if (this->address != nullptr)
        {
            free(this->address);
        }

        this->address = strdup(address);
#endif
        this->port = port;
        this->connectTimeout = connectTimeout;

        // Arduino clients return 1 on success and 0 or a negative code on failure;
        // PicoTcpClient and the Linux test client follow the same rule
        if (client->connect(address, port) <= 0)
        {
            setClientConnectionState(ConnectionState::DISCONNECTED);
            DEBUG("Communication Client failed to connect.\n");
            // TODO: Error handle
            return -1;
        }

        DEBUG("Client connect success\n");

        setClientConnectionState(ConnectionState::CONNECTING);

        elapsedTime.elapsed();
        return 0;
    }

    int MqttClient::disconnect(ReasonCode reasonCode)
    {
        if (!connected())
        {
            return -1;
        }
        Disconnect disconnect;
        disconnect.setReasonCode(reasonCode);
        sendPacket(&disconnect);
        setConnectionState(ConnectionState::DISCONNECTED);
        // MQTT 5: the sender of a DISCONNECT must close the Network Connection
        client->stop();
        setClientConnectionState(ConnectionState::DISCONNECTED);
        if (handler)
        {
            handler->onDisconnection(reasonCode);
        }
        return 0;
    }

    void MqttClient::addSubscribePayload(Subscribe &packet, SubscribePayload &payload)
    {
        addSubscriptionPayload((Subscription *)&packet, (SubscriptionPayload *)&payload);
    }

    void MqttClient::addUnsubscribePayload(Unsubscribe &packet, UnsubscribePayload &payload)
    {
        addSubscriptionPayload((Subscription *)&packet, (SubscriptionPayload *)&payload);
    }

    void MqttClient::addSubscriptionPayload(Subscription *packet, SubscriptionPayload *payload)
    {
        packet->addPayload(payload);
    }

    int MqttClient::sendSubscription(Subscription &packet)
    {
        Token identifier = getPacketIdentifier();
        packet.setPacketIdentifier(identifier);
        if (sendPacket(&packet, CPP_MQTT_TRANSMIT_RESERVE) < 0)
        {
            return -1;
        }
        return identifier;
    }

    void MqttClient::sync()
    {
#if defined(ARDUINO) || defined(ESP8266) || defined(ESP32)
#else
        client->sync();
#endif
        uint32_t elapsed = getElapsed();

        if (client->connected())
        {
            DEBUG("CLIENT IS CONNECTED\n");
            if (connectionState == +ConnectionState::DISCONNECTED)
            {
                DEBUG("mqttConnect\n");
                mqttConnect();
            }
            else
            {
                updateKeepAlivePeriod(elapsed);
                // A keep alive timeout closes the connection; nothing more to read
                if (client->connected())
                {
                    readNextPacket();
                }
            }

            if (connectionState == +ConnectionState::CONNECTED)
            {
                while (qosZeroFailed.size() > 0)
                {
                    auto identifier = qosZeroFailed.back();
                    qosZeroFailed.pop_back();
                    if (handler)
                    {
                        handler->onDeliveryFailure(identifier, ReasonCode::UNSPECIFIED_ERROR);
                    }
                }

                while (qosZeroSuccess.size() > 0)
                {
                    auto identifier = qosZeroSuccess.back();
                    qosZeroSuccess.pop_back();
                    if (handler)
                    {
                        handler->onDeliveryComplete(identifier);
                    }
                }
            }
        }
        else
        {
            DEBUG("CLIENT IS NOT CONNECTED\n");
            if (connectionState == +ConnectionState::CONNECTED)
            {
                setClientConnectionState(ConnectionState::DISCONNECTED);
                setConnectionState(ConnectionState::DISCONNECTED);
                if (handler)
                {
                    handler->onDisconnection(ReasonCode::UNSPECIFIED_ERROR);
                }
            }
            else if (clientState == +ConnectionState::CONNECTING)
            {
                if (elapsed > this->connectTimeout)
                {
                    setConnectionState(ConnectionState::DISCONNECTED);
                    setClientConnectionState(ConnectionState::DISCONNECTED);
                    client->stop();
                    if (handler)
                    {
                        handler->onDisconnection(ReasonCode::UNSPECIFIED_ERROR);
                    }
                }
                else
                {
                    this->connectTimeout -= elapsed;
                }
            }
        }
    }

    int MqttClient::sendPacket(Packet *packet, size_t reserve)
    {
        if (transmitBuffer != NULL)
        {
            size_t packetLength = packet->totalSize();
            if (transmitBuffer->writable() < packetLength + reserve)
            {
                // Not enough room: nothing is written, so the connection stays usable
                DEBUG("Transmit buffer full; packet not sent\n");
                return -1;
            }
            PacketBuffer packetBuffer(transmitBuffer, packetLength);
            packet->push(packetBuffer);
            // Commits only a complete packet, so a failure here sends nothing
            return packetBuffer.flush() ? (int)packetLength : -1;
        }

        // Streamed to the socket in small chunks: the packet is never assembled
        // in memory, so sending needs no allocation and little stack
        size_t length = packet->totalSize();
        PacketBuffer packetBuffer(client, length);
        packet->push(packetBuffer);
        bool complete = packetBuffer.flush() && packetBuffer.getLength() == length;

        if (!complete)
        {
            if (packetBuffer.getLength() > 0)
            {
                // Part of a packet has been sent, so the stream can no longer be
                // framed by the broker; the connection has to be dropped
                DEBUG("Packet write failed part way; closing connection\n");
                client->stop();
                setClientConnectionState(ConnectionState::DISCONNECTED);
                setConnectionState(ConnectionState::DISCONNECTED);
            }
            return -1;
        }
        return (int)length;
    }

    void MqttClient::updateKeepAlivePeriod(uint32_t timeElapsed)
    {
        if (timeElapsed > 0 && getKeepAliveInterval() > 0)
        {
            if (serverKeepAliveTimeRemaining <= timeElapsed)
            {
                disconnect(ReasonCode::KEEP_ALIVE_TIMEOUT);
            }
            else
            {
                serverKeepAliveTimeRemaining -= timeElapsed;
            }

            if (clientKeepAliveTimeRemaining <= timeElapsed)
            {

                if (connectionState == +ConnectionState::CONNECTED)
                {
                    ping();
                }
            }
            else
            {
                clientKeepAliveTimeRemaining -= timeElapsed;
            }
        }
    }

    void MqttClient::setClientConnectionState(ConnectionState state)
    {
        clientState = state;
    }

    void MqttClient::setConnectionState(ConnectionState state)
    {
        connectionState = state;
    }

    void MqttClient::setCleanStart(bool value)
    {
        connectPacket.setCleanStart(value);
    }

    bool MqttClient::connected()
    {
        if (client->connected())
        {
            return connectionState == +ConnectionState::CONNECTED;
            // TODO: Recheck connection timeout
        }
        return false;
    }

    void MqttClient::setHandler(MqttClientHandler *hander)
    {
        this->handler = hander;
    }

    uint16_t MqttClient::getPacketIdentifier()
    {
        return ++packetIdentifier ? packetIdentifier : ++packetIdentifier;
    }

    bool MqttClient::isDelivered(uint16_t token)
    {
        return !hasClientToken(token);
    }

    void MqttClient::setClientId(EncodedString &id)
    {
        connectPacket.setClientId(id);
    }

    void MqttClient::setClientId(const char *data, uint16_t length)
    {
        connectPacket.setClientId(data, length);
    }

    void MqttClient::ping()
    {
        clientKeepAliveTimeRemaining = getKeepAliveInterval() * SECONDS_TO_MS;
        PingRequest ping;
        sendPacket(&ping);
    }

    void MqttClient::pingResponse()
    {
        // TODO: Ping Reponse
    }

    void MqttClient::messageReceived(EncodedString &topic, Payload &payload)
    {
        if (handler)
        {
            handler->onMessage(topic, payload);
        }
    }

    void MqttClient::connectionAcknowledged(ConnectAcknowledge *packet)
    {
        uint8_t reasonCode = packet->getReasonCode();

        if (reasonCode != 0)
        {
            if (handler)
            {
                handler->onConnectionFailure(reasonCode);
            }
            return;
        }

        // A Server Keep Alive replaces the client's requested interval
        if (packet->getServerKeepAlive() > 0)
        {
            connectPacket.setKeepAliveInterval(packet->getServerKeepAlive());
        }

        // Start both timers from the interval now in force. readNextPacket set the
        // server timer before this CONNACK could change the interval, which left
        // it at 0 when the client had requested no keep alive.
        serverKeepAliveTimeRemaining = (uint32_t)getKeepAliveInterval() * KEEP_ALIVE_TIMEOUT_MS_PER_SECOND;
        clientKeepAliveTimeRemaining = (uint32_t)getKeepAliveInterval() * SECONDS_TO_MS;

        setConnectionState(ConnectionState::CONNECTED);

        if (handler)
        {
            handler->onConnectionSuccess();
        }
    }

    void MqttClient::onPublish(Publish *packet)
    {
        Token identifier = packet->getPacketIdentifier();
        // Validate Packet

        QoS qos = packet->getQos();

        switch (qos)
        {
        case QoS::ZERO:
        {
            messageReceived(packet->getTopic(), packet->getPayload());
        }
        break;
        case QoS::ONE:
        {
            messageReceived(packet->getTopic(), packet->getPayload());
            PublishAcknowledge acknowledge;
            acknowledge.setPacketIdentifier(identifier);
            acknowledge.setReasonCode(0);
            sendPacket(&acknowledge);
        }
        break;
        case QoS::TWO:
        {
            // Deliver once, on first receipt. A retransmitted PUBLISH with the
            // same identifier (before PUBREL) is acknowledged but not delivered again.
            if (find(serverTokens.begin(), serverTokens.end(), identifier) == serverTokens.end())
            {
                // Receive Maximum keeps a conforming broker within the fixed list
                pushBack(serverTokens, identifier);
                messageReceived(packet->getTopic(), packet->getPayload());
            }

            PublishReceived received;
            received.setPacketIdentifier(identifier);
            received.setReasonCode(0);
            sendPacket(&received);
        }
        break;
        default:
            break;
        }

        // TODO: Token Success
    }

    void MqttClient::onPublishAcknowledge(PublishAcknowledge *packet)
    {
        uint16_t identifier = packet->getPacketIdentifier();

        if (hasClientToken(identifier))
        {
            if (packet->getReasonCode() == 0)
            {
                // TODO: Token Failure
                if (handler)
                {
                    handler->onDeliveryComplete(identifier);
                }
                // TODO: Token Success
            }
            else
            {
                // TODO: Token Failure
                if (handler)
                {
                    handler->onDeliveryFailure(identifier, packet->getReasonCode());
                }
            }
            removeClientToken(identifier);
        }
    }

    void MqttClient::onPublishReceived(PublishReceived *packet)
    {
        uint16_t identifier = packet->getPacketIdentifier();

        if (hasClientToken(identifier))
        {
            if (packet->getReasonCode() == 0)
            {
                // TODO: Token Success
                PublishRelease release;
                release.setPacketIdentifier(identifier);
                release.setReasonCode(0);
                sendPacket(&release);
            }
            else
            {
                // TODO: Token Failure
                if (handler)
                {
                    handler->onDeliveryFailure(identifier, packet->getReasonCode());
                }
                // TODO: Token Failure
                removeClientToken(identifier);
            }
        }
    }

    void MqttClient::onPublishRelease(PublishRelease *packet)
    {
        Token identifier = packet->getPacketIdentifier();
        // Validate Packet

        auto pending = find(serverTokens.begin(), serverTokens.end(), identifier);
        if (pending != serverTokens.end())
        {
            serverTokens.erase(pending);
        }

        PublishComplete complete;
        complete.setPacketIdentifier(identifier);
        complete.setReasonCode(0);
        sendPacket(&complete);
    }

    void MqttClient::onPublishComplete(PublishComplete *packet)
    {
        uint16_t identifier = packet->getPacketIdentifier();

        if (hasClientToken(identifier))
        {
            if (packet->getReasonCode() == 0)
            {
                // TODO: Token Success
                if (handler)
                {
                    handler->onDeliveryComplete(identifier);
                }
            }
            else
            {
                // TODO: Token Failure
                if (handler)
                {
                    handler->onDeliveryFailure(identifier, packet->getReasonCode());
                }
            }
            removeClientToken(identifier);
        }
    }

    void MqttClient::mqttConnect()
    {
        DEBUG("Sending MQTT Connect\n");
        serverKeepAliveTimeRemaining = ((uint32_t)getKeepAliveInterval() * KEEP_ALIVE_TIMEOUT_MS_PER_SECOND);
        setConnectionState(ConnectionState::CONNECTING);
        sendPacket(&connectPacket);
    }

    ConnectionState MqttClient::getConnectionState()
    {
        return connectionState;
    }

    bool MqttClient::hasClientToken(uint16_t token)
    {
        for (auto &item : clientTokens)
        {
            if (item == token)
            {
                return true;
            }
        }
        return false;
    }

    void MqttClient::removeClientToken(uint16_t token)
    {
        clientTokens.erase(find(clientTokens.begin(), clientTokens.end(), token));
    }

    uint32_t MqttClient::getElapsed()
    {
        return elapsedTime.elapsed();
    }

    uint16_t MqttClient::getKeepAliveInterval()
    {
        return connectPacket.getKeepAliveInterval();
    }

    void MqttClient::setKeepAliveInterval(uint16_t value)
    {
        connectPacket.setKeepAliveInterval(value);
    }

    uint32_t MqttClient::getSessionExpiryInterval()
    {
        return 0;
    }

    // TODO: Implement
    void MqttClient::setSessionExpiryInterval(__attribute__((unused)) uint32_t value)
    {
    }

    uint16_t MqttClient::getReceiveMaximum()
    {
        return 0;
    }

    // TODO: Implement
    void MqttClient::setReceiveMaximum(__attribute__((unused)) uint16_t value)
    {
    }

    uint32_t MqttClient::getMaximumPacketSize()
    {
        return 0;
    }

    // TODO: Implement
    void MqttClient::setMaximumPacketSize(__attribute__((unused)) uint32_t value)
    {
    }

    uint16_t MqttClient::getTopicAliasMaximum()
    {
        return 0;
    }

    // TODO: Implement
    void MqttClient::setTopicAliasMaximum(__attribute__((unused)) uint16_t value)
    {
    }

    EncodedString MqttClient::getUserName()
    {
        return EncodedString();
    }

    // TODO: Implement
    void MqttClient::setUserName(__attribute__((unused)) EncodedString value)
    {
    }

    EncodedString MqttClient::getPassword()
    {
        return EncodedString();
    }

    // TODO: Implement
    void MqttClient::setPassword(__attribute__((unused)) EncodedString value)
    {
    }

    void MqttClient::setAutoReconnect(int32_t value)
    {
        autoReconnect = value;
    }

    int MqttClient::getAutoReconnect()
    {
        return autoReconnect;
    }

    uint16_t MqttClient::publish(EncodedString &topic, Payload &payload, QoS qos, bool retain)
    {
        if (!connected())
        {
            return PUBLISH_FAILED;
        }

        // With STATIC_MEMORY the result of every publish must be tracked in a
        // fixed list; refuse rather than lose the delivery callback
        if ((qos == +QoS::ZERO) ? (isFull(qosZeroSuccess) || isFull(qosZeroFailed)) : isFull(clientTokens))
        {
            return PUBLISH_FAILED;
        }

        Publish publishPacket;

        publishPacket.setTopic(topic);
        // Sent straight from the caller's memory, without a copy
        publishPacket.setPayloadReference(payload.getData(), payload.size());
        publishPacket.setQos(qos);
        publishPacket.setRetain(retain);

        uint16_t packetIdentifier = getPacketIdentifier();

        if (qos != +QoS::ZERO)
        {
            publishPacket.setPacketIdentifier(packetIdentifier);
        }

        auto result = sendPacket(&publishPacket, CPP_MQTT_TRANSMIT_RESERVE);

        if (qos == +QoS::ZERO)
        {
            // TODO: Implement feedback from when the TCP Client succeeds in sending messages
            if (result >= 0)
            {
                pushBack(qosZeroSuccess, packetIdentifier);
            }
            else
            {
                pushBack(qosZeroFailed, packetIdentifier);
            }
        }
        else
        {
            if (result < 0)
            {
                // Never sent, so no acknowledgement will arrive to complete the token
                return PUBLISH_FAILED;
            }
            pushBack(clientTokens, packetIdentifier);
        }

        return packetIdentifier;
    }
}