/*
 * File: MqttClient.h
 * Project: cpp_mqtt_client
 * Created Date: Friday December 30th 2022
 * Author: Kyle Hofer
 *
 * MIT License
 *
 * Copyright (c) 2022 Kyle Hofer
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

#ifndef MQTTCLIENT
#define MQTTCLIENT

#include "properties/WillProperties.h"
#include "types/SubscriptionPayload.h"
#include "types/Payload.h"
#include "types/EncodedString.h"
#include "packets/Connect.h"
#include "packets/Packet.h"
#include "packets/ConnectAcknowledge.h"
#include "packets/Disconnect.h"
#include <functional>
#include <map>
#include "Client.h"
#include "packets/PacketUtility.h"
#include "types/Common.h"

#ifdef __linux__
#include <iostream>
#include <chrono>
#endif

using namespace std;

namespace PicoMqtt
{
    typedef function<void(Packet *)> packetResponse;

    class MqttClient
    {
    private:
        Client *client;
        WillProperties *willProperties;
        Connect connectPacket;
        bool awaitingPingResponse = false;
        // Original idea was to use a hashmap of responses to packets.
        // However hard coded responses would be more practical
        map<uint8_t, packetResponse> responses;
        bool isAcknowledged = false;
        bool waitingAcknowledge = false;
        bool disconnecting = false;

        /* Connect and Acknowledge properties */
        EncodedString username;
        EncodedString password;

        uint32_t willDelayInterval;
        uint32_t messageExpiryInterval;
        uint32_t sessionExpiryInterval;
        uint16_t receiveMaximum;
        uint32_t maximumPacketSize;
        uint16_t topicAliasMaximum;
        uint16_t packetIdentifier = 0;

        /* Timers */
        uint32_t clientKeepAliveTimeRemaining = 0;
        uint32_t serverKeepAliveTimeRemaining = 0;

        uint8_t state = 0;
#if defined(PICO)
        uint64_t lastExecutionTime = 0;
#elif defined(ARDUINO)
        uint32_t lastExecutionTime = 0;
#elif defined(__linux__)
        std::chrono::_V2::system_clock::time_point lastExecutionTime;
#endif
        vector<uint16_t> clientTokens;
        vector<uint16_t> serverTokens;

        /**
         * @brief Attempts to read the next MQTT Packet
         *
         * @return Packet* The packet read. Returns NULL if a complete packet has not been read
         */
        Packet *readNextPacket();
        void ping();
        void pingResponse();
        void messageReceived(EncodedString &topic, Payload &payload);
        void connectionAcknowledged(ConnectAcknowledge *packet);
        void mqttDisconnected(Disconnect *packet);

        void onPublish(Publish *packet);
        void onPublishAcknowledge(PublishAcknowledge *packet);
        void onPublishReceived(PublishReceived *packet);
        void onPublishRelease(PublishRelease *packet);
        void onPublishComplete(PublishComplete *packet);

        /**
         * @brief Whether the client has the token in its list of active tokens
         *
         * @param token
         * @return true
         * @return false
         */
        bool hasClientToken(uint16_t token);
        /**
         * @brief Removes a token from the list of active tokens
         *
         * @param token
         */
        void removeClientToken(uint16_t token);

        /**
         * @brief Get the next unique packet identifier
         * Also known as a packet token
         *
         * @return uint16_t
         */
        uint16_t getPacketIdentifier();

        /**
         * @brief Sends a packet through the client
         *
         * @param packet
         */
        void sendPacket(Packet *packet);

        /**
         * @brief Updates the keep alive period timers
         * Will handle disconnections or ping requests based off inactivity
         *
         * @param timeElapsed
         */
        void updateKeepAlivePeriod(uint32_t timeElapsed);

    protected:
        /**
         * @brief Return the elapsed time since the last call
         *
         * @return uint32_t elapsed time in milliseconds
         */
        virtual uint32_t getElapsed();

    public:
        MqttClient();
        template <typename CommunicationClient>
        MqttClient();
        MqttClient(Client *client);
        int setWill(WillProperties *);
        int connect(const char *address, int port, uint32_t connectTimeout);
        int disconnect(uint8_t reasonCode);
        int subscribe(SubscriptionPayload &payload...);
        int unsubscribe(SubscriptionPayload &payload...);
        void sync();
        void setCleanStart(bool value);
        void setMaxInflightMessages();
        void setCredentials(EncodedString name, EncodedString password);
        bool connected();

        /* Utilities */
        /**
         * @brief Whether a token has been delivered
         *
         * @param token
         * @return true
         * @return false
         */
        bool isDelivered(uint16_t token);

        /* Client properties Getters/Setters */
        uint16_t getKeepAliveInterval();
        void setKeepAliveInterval(uint16_t value);
        uint32_t getWillDelayInterval();
        void setWillDelayInterval(uint32_t value);
        uint32_t getMessageExpiryInterval();
        void setMessageExpiryInterval(uint32_t value);
        uint32_t getSessionExpiryInterval();
        void setSessionExpiryInterval(uint32_t value);
        uint16_t getReceiveMaximum();
        void setReceiveMaximum(uint16_t value);
        uint32_t getMaximumPacketSize();
        void setMaximumPacketSize(uint32_t value);
        uint16_t getTopicAliasMaximum();
        void setTopicAliasMaximum(uint16_t value);
        EncodedString getUserName();
        void setUserName(EncodedString value);
        EncodedString getPassword();
        void setPassword(EncodedString value);

        /* Publish Actions */
        /**
         * @brief Publish a payload over MQTT
         *
         * @param topic The topic to publish the payload with
         * @param payload The payload to publish
         * @param qos The QOS of the payload to publish
         * @return uint16_t The unique token used to identify a publish packet
         */
        uint16_t publish(EncodedString &topic, Payload &payload, QoS qos);

        /* Subscribe Actions */
    };
}

#endif /* MQTTCLIENT */
