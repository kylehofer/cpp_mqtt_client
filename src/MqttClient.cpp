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

#ifdef DEBUGGING
#define DEBUG(format, ...)  \
    printf("MqttClient: "); \
    printf(format, ##__VA_ARGS__)
#else
#define DEBUG(out, ...)
#endif

using namespace std;

#define SECONDS_TO_MS 1000
// MQTT 5 Specifies that that 1.5 times the Keep Alive Time is limit of the Keep Alive Timeout
#define KEEP_ALIVE_SCALER 1.5 * SECONDS_TO_MS

namespace PicoMqtt
{

    Packet *MqttClient::readNextPacket()
    {
        Packet *packet = readPacketFromClient(client);

        if (packet == NULL)
        {
            return NULL;
        }

        serverKeepAliveTimeRemaining = (getKeepAliveInterval() * KEEP_ALIVE_SCALER);

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

        delete packet;

        return NULL;
    }

    MqttClient::MqttClient()
    {
    }

    template <typename CommunicationClient>
    MqttClient::MqttClient()
    {
        client = (Client *)new CommunicationClient();
    }

    MqttClient::MqttClient(Client *client)
    {
        this->client = client;
    }

    MqttClient::~MqttClient()
    {
        if (this->address)
        {
            free(this->address);
        }
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

        if (this->address != nullptr)
        {
            free(this->address);
        }

        this->address = strdup(address);
        this->port = port;
        this->connectTimeout = connectTimeout;

        if (client->connect(address, port) != 0)
        {
            setClientConnectionState(ConnectionState::DISCONNECTED);
            DEBUG("Communication Client failed to connect.\n");
            // TODO: Error handle
            return -1;
        }

        setClientConnectionState(ConnectionState::CONNECTING);

#if defined(PICO)
        lastExecutionTime = time_us_64();
#elif defined(ARDUINO) || defined(ESP8266) || defined(ESP32)
        uint32_t now = millis();
        lastExecutionTime = millis();
#elif defined(__linux__)
        lastExecutionTime = std::chrono::system_clock::now();
#endif
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
        if (handler)
        {
            handler->onDisconnection(reasonCode);
        }
        return 0;
    }

    template <typename... T>
    void MqttClient::addSubscribePayload(Subscribe &packet, SubscribePayload &payload, T &...args)
    {
        addSubscribePayload(packet, payload);
        addSubscribePayload(packet, args...);
    }

    void MqttClient::addSubscribePayload(Subscribe &packet, SubscribePayload &payload)
    {
        addSubscriptionPayload((Subscription *)&packet, (SubscriptionPayload *)&payload);
    }

    template <typename... T>
    void MqttClient::addUnsubscribePayload(Unsubscribe &packet, UnsubscribePayload &payload, T &...args)
    {
        addUnsubscribePayload(packet, payload);
        addUnsubscribePayload(packet, args...);
    }

    void MqttClient::addUnsubscribePayload(Unsubscribe &packet, UnsubscribePayload &payload)
    {
        addSubscriptionPayload((Subscription *)&packet, (SubscriptionPayload *)&payload);
    }

    void MqttClient::addSubscriptionPayload(Subscription *packet, SubscriptionPayload *payload)
    {
        packet->addPayload(payload);
    }

    int MqttClient::subscribe(SubscribePayload &payload...)
    {
        if (!connected())
        {
            return -1;
        }

        Token identifier = getPacketIdentifier();

        Subscribe packet;

        addSubscribePayload(packet, payload);

        packet.setPacketIdentifier(identifier);

        sendPacket(&packet);

        return identifier;
    }

    int MqttClient::unsubscribe(UnsubscribePayload &payload...)
    {
        if (!connected())
        {
            return -1;
        }

        Token identifier = getPacketIdentifier();

        Unsubscribe packet;

        addUnsubscribePayload(packet, payload);

        packet.setPacketIdentifier(identifier);

        sendPacket(&packet);

        return identifier;
    }

    void MqttClient::sync()
    {
        client->sync();
        uint32_t elapsed = getElapsed();

        if (client->connected())
        {
            if (connectionState == +ConnectionState::DISCONNECTED)
            {
                mqttConnect();
            }
            else
            {
                updateKeepAlivePeriod(elapsed);
                readNextPacket();
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
            if (connectionState == +ConnectionState::CONNECTED)
            {
                setClientConnectionState(ConnectionState::DISCONNECTED);
                setConnectionState(ConnectionState::DISCONNECTED);
                handler->onDisconnection(ReasonCode::UNSPECIFIED_ERROR);
            }
            else if (clientState == +ConnectionState::CONNECTING)
            {
                if (elapsed > this->connectTimeout)
                {
                    setConnectionState(ConnectionState::DISCONNECTED);
                    setClientConnectionState(ConnectionState::DISCONNECTED);
                    client->stop();
                    handler->onDisconnection(ReasonCode::UNSPECIFIED_ERROR);
                }
                else
                {
                    this->connectTimeout -= elapsed;
                }
            }
        }
    }

    int MqttClient::sendPacket(Packet *packet)
    {
        PacketBuffer packetBuffer(packet->totalSize());
        packet->push(packetBuffer);

        return client->write(packetBuffer.getBuffer(), packetBuffer.getLength());
    }

    void MqttClient::updateKeepAlivePeriod(uint32_t timeElapsed)
    {
        if (getKeepAliveInterval() > 0)
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

        setConnectionState(ConnectionState::CONNECTED);

        if (handler)
        {
            handler->onConnectionSuccess();
        }

        if (packet->getServerKeepAlive() > 0)
        {
            connectPacket.setKeepAliveInterval(packet->getServerKeepAlive());
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
            if (publishQueue.contains(identifier))
                delete publishQueue[identifier];

            publishQueue[identifier] = new Publish(*packet);
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

        if (publishQueue.contains(identifier))
        {
            Publish *publish = publishQueue[identifier];
            messageReceived(publish->getTopic(), publish->getPayload());
            delete publish;
            publishQueue.erase(identifier);
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
        serverKeepAliveTimeRemaining = (getKeepAliveInterval() * KEEP_ALIVE_SCALER);
        setConnectionState(ConnectionState::CONNECTING);
        sendPacket(&connectPacket);
    }

    ConnectionState MqttClient::getConnectionState()
    {
        return connectionState;
    }

    bool MqttClient::hasClientToken(uint16_t token)
    {
        auto item = ranges::find_if(clientTokens.begin(), clientTokens.end(),
                                    [token](uint16_t item)
                                    { return item == token; });
        return item != clientTokens.end();
    }

    void MqttClient::removeClientToken(uint16_t token)
    {
        clientTokens.erase(find(clientTokens.begin(), clientTokens.end(), token));
    }

    uint32_t MqttClient::getElapsed()
    {

#if defined(PICO)
        uint64_t now = time_us_64();
        uint64_t duration;
        if (now < lastExecutionTime)
        {
            duration = (now + (0xFFFFFFFF ^ lastExecutionTime)) / 1000;
        }
        else
        {
            duration = (now - lastExecutionTime) / 1000;
        }
        lastExecutionTime = now;
#elif defined(ARDUINO) || defined(ESP8266) || defined(ESP32)
        uint32_t now = millis();
        uint32_t duration;
        if (now < lastExecutionTime)
        {
            duration = (now + (0xFFFF ^ lastExecutionTime));
        }
        else
        {
            duration = (now - lastExecutionTime);
        }
        lastExecutionTime = now;
#elif defined(__linux__)
        const auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastExecutionTime).count();
        lastExecutionTime = now;

#endif

        return duration;
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
            return -1;
        }

        Publish publishPacket;

        publishPacket.setTopic(topic);
        publishPacket.setPayload(payload);
        publishPacket.setQos(qos);
        publishPacket.setRetain(retain);

        uint16_t packetIdentifier = getPacketIdentifier();

        if (qos != +QoS::ZERO)
        {
            publishPacket.setPacketIdentifier(packetIdentifier);
        }

        auto result = sendPacket(&publishPacket);

        if (qos == +QoS::ZERO)
        {
            // TODO: Implement feedback from when the TCP Client succeeds in sending messages
            if (result >= 0)
            {
                qosZeroSuccess.push_back(packetIdentifier);
            }
            else
            {
                qosZeroFailed.push_back(packetIdentifier);
            }
        }
        else
        {
            clientTokens.push_back(packetIdentifier);
        }

        return packetIdentifier;
    }
}