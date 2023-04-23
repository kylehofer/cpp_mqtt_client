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

using namespace std;

namespace PicoMqtt
{
    Packet *MqttClient::readNextPacket()
    {
        Packet *packet = readPacketFromClient(client);

        if (packet == NULL)
        {
            return NULL;
        }

        uint8_t packetType = packet->getPacketType();

        switch (packetType)
        {
        case CONNECT_ID:
            // TODO: Should never receieve
            break;
        case CONNECT_ACKNOWLEDGE_ID:
        {
            ConnectAcknowledge *connectAcknowledge = (ConnectAcknowledge *)packet;
            connectionAcknowledged(connectAcknowledge);
        }
        break;

        case PUBLISH_ID:
        {
            Publish *publish = (Publish *)packet;
            messageReceived(publish->getTopic(), publish->getPayload());
        }
        break;
        case PUBLISH_ACKNOWLEDGE_ID:
        {
            onPublishAcknowledge((PublishAcknowledge *)packet);
        }
        break;
        case PUBLISH_RECEIVED_ID:
            onPublishReceived((PublishReceived *)packet);
            break;
        case PUBLISH_RELEASE_ID:
            onPublishRelease((PublishRelease *)packet);
            break;
        case PUBLISH_COMPLETE_ID:
            onPublishComplete((PublishComplete *)packet);
            break;
        case SUBSCRIBE_ID:
            // TODO: Should never receieve
            break;
        case SUBSCRIBE_ACKNOWLEDGE_ID:
            break;
        case UNSUBSCRIBE_ID:
            // TODO: Should never receieve
            break;
        case UNSUBSCRIBE_ACKNOWLEDGE_ID:
            // TODO: Should never receieve
            break;
        case PING_REQUEST_ID:
        {
            PingResponse response;
            response.pushToClient(client);
            break;
        }
        case PING_RESPONSE_ID:
            pingResponse();
            break;
        case DISCONNECT_ID:

            break;
        case AUTHENTICATION_ID:
        default:
            break;
        }

        delete packet;

        return NULL;
    }

    // template <typename CommunicationClient>
    MqttClient::MqttClient()
    {
        // client = new CommunicationClient();
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

    int MqttClient::setWill(WillProperties *will)
    {
        connectPacket.setWill(will);
        return 0;
    }

    int MqttClient::connect(const char *address, int port, uint32_t connectTimeout)
    {
        if (client->connect(address, port))
        {
            // TODO: Error handle
        }

        if (!client->connected())
        {
            // TODO: Error handle
            return -1;
        }

        connectPacket.pushToClient(client);

        return 0;
    }

    int MqttClient::disconnect()
    {
        return 0;
    }

    int MqttClient::subscribe(SubscriptionPayload &payload...)
    {
        return 0;
    }

    int MqttClient::unsubscribe(SubscriptionPayload &payload...)
    {
        return 0;
    }

    void MqttClient::sync()
    {
        uint32_t elapsed = getElapsed();

        if (client->connected())
        {
            readNextPacket();
        }
        else
        {
            // TODO: Not Connected
        }
    }

    void MqttClient::setCleanStart(bool value)
    {
        connectPacket.setCleanStart(value);
    }

    bool MqttClient::connected()
    {
        if (client->connected())
        {
            if (isAcknowledged)
            {
                return true;
            }

            // TODO: Recheck connection timeout
        }
        return false;
        // return client->connected() && isAcknowledged;
    }

    uint16_t MqttClient::getPacketIdentifier()
    {
        return packetIdentifier++;
    }

    bool MqttClient::isDelivered(uint16_t token)
    {
        return !hasClientToken(token);
    }

    void MqttClient::ping()
    {
        PingRequest ping;
        ping.pushToClient(client);
    }

    void MqttClient::pingResponse()
    {
        // TODO: Ping Reponse
    }

    void MqttClient::messageReceived(EncodedString &topic, Payload &payload)
    {
    }

    void MqttClient::connectionAcknowledged(ConnectAcknowledge *packet)
    {
        uint8_t reasonCode = packet->getReasonCode();
        if (reasonCode != 0)
        {
            printf("Failed to Connect. Reason: %d\n", reasonCode);
            return;
        }
        // TODO: Connection Validation
        isAcknowledged = true;

        if (packet->getServerKeepAlive() > 0)
        {
            connectPacket.setKeepAliveInterval(packet->getServerKeepAlive());
        }
    }

    void MqttClient::mqttDisconnected(Disconnect *packet)
    {
    }

    void MqttClient::onPublish(Publish *packet)
    {
        uint16_t identifier = packet->getPacketIdentifier();
        // Validate Packet

        QoS qos = packet->getQos();

        switch (qos)
        {
        case QoS::ONE:
        {
            PublishAcknowledge acknowledge;
            acknowledge.setPacketIdentifier(identifier);
            acknowledge.setReasonCode(0);
            acknowledge.pushToClient(client);
        }
        break;
        case QoS::TWO:
        {
            PublishReceived received;
            received.setPacketIdentifier(identifier);
            received.setReasonCode(0);
            received.pushToClient(client);
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
                // TODO: Token Success
            }
            else
            {
                // TODO: Token Failure
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
                release.pushToClient(client);
            }
            else
            {
                // TODO: Token Failure
                removeClientToken(identifier);
            }
        }
    }

    void MqttClient::onPublishRelease(PublishRelease *packet)
    {
        uint16_t identifier = packet->getPacketIdentifier();
        // Validate Packet

        PublishComplete complete;
        complete.setPacketIdentifier(identifier);
        complete.setReasonCode(0);
        complete.pushToClient(client);
    }

    void MqttClient::onPublishComplete(PublishComplete *packet)
    {
        uint16_t identifier = packet->getPacketIdentifier();

        if (hasClientToken(identifier))
        {
            if (packet->getReasonCode() == 0)
            {
                // TODO: Token Success
            }
            else
            {
                // TODO: Token Failure
            }
            removeClientToken(identifier);
        }
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
#elif defined(__linux__)
        const auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastExecutionTime).count();
        lastExecutionTime = now;

#endif

        return duration;
    }

    uint16_t MqttClient::getKeepAliveInterval()
    {
        return 0;
    }

    void MqttClient::setKeepAliveInterval(uint16_t value)
    {
        connectPacket.setKeepAliveInterval(value);
    }

    uint32_t MqttClient::getWillDelayInterval()
    {
        return connectPacket.getKeepAliveInterval();
    }

    void MqttClient::setWillDelayInterval(uint32_t value)
    {
    }

    uint32_t MqttClient::getMessageExpiryInterval()
    {
        return 0;
    }

    void MqttClient::setMessageExpiryInterval(uint32_t value)
    {
    }

    uint32_t MqttClient::getSessionExpiryInterval()
    {
        return 0;
    }

    void MqttClient::setSessionExpiryInterval(uint32_t value)
    {
    }

    uint16_t MqttClient::getReceiveMaximum()
    {
        return 0;
    }

    void MqttClient::setReceiveMaximum(uint16_t value)
    {
    }

    uint32_t MqttClient::getMaximumPacketSize()
    {
        return 0;
    }

    void MqttClient::setMaximumPacketSize(uint32_t value)
    {
    }

    uint16_t MqttClient::getTopicAliasMaximum()
    {
        return 0;
    }

    void MqttClient::setTopicAliasMaximum(uint16_t value)
    {
    }

    EncodedString MqttClient::getUserName()
    {
        return EncodedString();
    }

    void MqttClient::setUserName(EncodedString value)
    {
    }

    EncodedString MqttClient::getPassword()
    {
        return EncodedString();
    }

    void MqttClient::setPassword(EncodedString value)
    {
    }

    uint16_t MqttClient::publish(EncodedString &topic, Payload &payload, QoS qos)
    {
        Publish publishPacket;

        publishPacket.setTopic(topic);
        publishPacket.setPayload(payload);
        publishPacket.setQos(qos);

        uint16_t packetIdentifier = getPacketIdentifier();

        publishPacket.setPacketIdentifier(packetIdentifier);

        if (qos != QoS::ZERO)
        {
            clientTokens.push_back(packetIdentifier);
        }

        publishPacket.pushToClient(client);

        return packetIdentifier;
    }
}