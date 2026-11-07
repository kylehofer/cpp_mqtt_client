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

#ifndef SRC_MQTTCLIENT
#define SRC_MQTTCLIENT

#include "CppMqttConfig.h"
#include "properties/WillProperties.h"
#include "types/SubscribePayload.h"
#include "types/UnsubscribePayload.h"
#include "types/Payload.h"
#include "types/EncodedString.h"
#include "packets/Connect.h"
#include "packets/Packet.h"
#include "packets/ConnectAcknowledge.h"
#include "packets/Disconnect.h"
#include <type_traits>
#include "SocketClient.h"
#include "TransmitBuffer.h"
#include "packets/PacketUtility.h"
#include "types/Common.h"
#include "utils/enum.h"

#include <ElapsedTime.h>

#ifdef __linux__
#include <iostream>
#include <chrono>
#endif

#ifdef PICO
#include "pico/stdlib.h"
#endif


namespace CppMqtt
{
    typedef uint16_t Token;
    // Returned by publish() on failure. MQTT never uses 0 as a Packet Identifier.
    constexpr Token PUBLISH_FAILED = 0;

    BETTER_ENUM(ConnectionState, uint8_t,
                DISCONNECTED,
                CONNECTED,
                CONNECTING)

    /**
     * @brief A handler for receiving callbacks from the MQTT Client
     *
     */
    class MqttClientHandler
    {
    public:
        virtual void onConnectionSuccess() = 0;
        virtual void onConnectionFailure(int reasonCode) = 0;
        virtual void onDisconnection(ReasonCode reasonCode) = 0;
        virtual void onMessage(EncodedString &topic, Payload &payload) = 0;
        virtual void onDeliveryComplete(Token token) = 0;
        virtual void onDeliveryFailure(Token token, int reasonCode) = 0;
        virtual void onSubscribeResult(Token token, ReasonCodes reasonCodes) = 0;
        virtual void onUnsubscribeResult(Token token, ReasonCodes reasonCodes) = 0;
    };

    class MqttClient
    {
    private:
        WillProperties *willProperties = NULL;
        Connect connectPacket;
        bool awaitingPingResponse = false;
        ConnectionState connectionState = ConnectionState::DISCONNECTED;
        ConnectionState clientState = ConnectionState::DISCONNECTED;

        /* Connect and Acknowledge properties */
        EncodedString username;
        EncodedString password;

        MqttClientHandler *handler = NULL;

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

        int32_t autoReconnect = -1;

#ifdef STATIC_MEMORY
        char address[CPP_MQTT_MAX_ADDRESS_LENGTH + 1] = {0};
#else
        char *address = nullptr;
#endif
        int port = -1;
        uint32_t connectTimeout = -1;
        uint32_t reconnectTimer = 0;
        bool attemptingReconnect = false;

        ElapsedTime elapsedTime;

        // Parse state for packets arriving on this connection
        PacketReader reader;

        Vector<uint16_t, CPP_MQTT_MAX_INFLIGHT> qosZeroFailed;
        Vector<uint16_t, CPP_MQTT_MAX_INFLIGHT> qosZeroSuccess;
        Vector<uint16_t, CPP_MQTT_MAX_INFLIGHT> clientTokens;
        // Packet Identifiers of received QoS 2 publishes awaiting a PUBREL.
        // The message is delivered on PUBLISH; the identifier is kept only to
        // suppress duplicate delivery until the PUBREL arrives.
        Vector<uint16_t, CPP_MQTT_MAX_INFLIGHT> serverTokens;

        void addSubscribePayload(Subscribe &packet, SubscribePayload &payload);
        void addUnsubscribePayload(Unsubscribe &packet, UnsubscribePayload &payload);

        /**
         * @brief Assigns a packet identifier to a SUBSCRIBE or UNSUBSCRIBE and sends it
         *
         * @return int The packet identifier used as the token
         */
        int sendSubscription(Subscription &packet);

        // When set, packets are encoded straight into the network client's memory
        TransmitBuffer *transmitBuffer = NULL;

        void addSubscriptionPayload(Subscription *packet, SubscriptionPayload *payload);

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

        void onPublish(Publish *packet);
        void onPublishAcknowledge(PublishAcknowledge *packet);
        void onPublishReceived(PublishReceived *packet);
        void onPublishRelease(PublishRelease *packet);
        void onPublishComplete(PublishComplete *packet);

        void mqttConnect();

        ConnectionState getConnectionState();

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
         * @param reserve With a TransmitBuffer, bytes that must stay free after this
         * packet; the packet is not sent if they would not
         * @return int The packet's length, or -1 if it was not sent
         */
        int sendPacket(Packet *packet, size_t reserve = 0);

        /**
         * @brief Updates the keep alive period timers
         * Will handle disconnections or ping requests based off inactivity
         *
         * @param timeElapsed
         */
        void updateKeepAlivePeriod(uint32_t timeElapsed);

        void setConnectionState(ConnectionState state);

        /**
         * @brief With STATIC_MEMORY, tells the broker the client's fixed limits in
         * the CONNECT packet so it never sends more than the buffers can hold
         */
        void applyStaticLimits();
        void setClientConnectionState(ConnectionState state);

    protected:
        Client *client = NULL;
        /**
         * @brief Return the elapsed time since the last call
         *
         * @return uint32_t elapsed time in milliseconds
         */
        // Virtual so tests can supply a controlled clock (see MockMqttClient)
        virtual uint32_t getElapsed();

    public:
        MqttClient();
        MqttClient(Client *client);

        /**
         * @brief A client whose packets are encoded straight into the network
         * client's transmit buffer (often the same object as client)
         */
        MqttClient(Client *client, TransmitBuffer *transmitBuffer);

        /**
         * @brief Encodes packets straight into the network client's memory instead
         * of calling Client::write(). NULL returns to Client::write().
         */
        void setTransmitBuffer(TransmitBuffer *transmitBuffer);
        ~MqttClient();

        // Not copyable: it owns a connection's parse state and packet storage.
        // Construct it in place (for example with emplace()) instead.
        MqttClient(const MqttClient &) = delete;
        MqttClient &operator=(const MqttClient &) = delete;
        int setWill(WillProperties *);
        int connect(const char *address, int port, uint32_t connectTimeout);
        int disconnect(ReasonCode reasonCode);
        /**
         * @brief Subscribes to one or more topics in a single SUBSCRIBE packet
         *
         * @param payload The first topic and its options
         * @param others Further SubscribePayloads to include in the same packet
         * @return int The token for the subscription, or -1 if not connected
         */
        template <typename... T>
        int subscribe(SubscribePayload &payload, T &...others)
        {
            static_assert((std::is_base_of_v<SubscribePayload, T> && ...),
                          "subscribe() only takes SubscribePayload arguments");
#ifdef STATIC_MEMORY
            static_assert(sizeof...(T) + 1 <= CPP_MQTT_MAX_SUBSCRIPTIONS,
                          "More topics than CPP_MQTT_MAX_SUBSCRIPTIONS in one packet");
#endif
            if (!connected())
            {
                return -1;
            }
            Subscribe packet;
            addSubscribePayload(packet, payload);
            (addSubscribePayload(packet, others), ...);
            return sendSubscription(packet);
        }

        /**
         * @brief Unsubscribes from one or more topics in a single UNSUBSCRIBE packet
         *
         * @param payload The first topic
         * @param others Further UnsubscribePayloads to include in the same packet
         * @return int The token for the unsubscription, or -1 if not connected
         */
        template <typename... T>
        int unsubscribe(UnsubscribePayload &payload, T &...others)
        {
            static_assert((std::is_base_of_v<UnsubscribePayload, T> && ...),
                          "unsubscribe() only takes UnsubscribePayload arguments");
#ifdef STATIC_MEMORY
            static_assert(sizeof...(T) + 1 <= CPP_MQTT_MAX_SUBSCRIPTIONS,
                          "More topics than CPP_MQTT_MAX_SUBSCRIPTIONS in one packet");
#endif
            if (!connected())
            {
                return -1;
            }
            Unsubscribe packet;
            addUnsubscribePayload(packet, payload);
            (addUnsubscribePayload(packet, others), ...);
            return sendSubscription(packet);
        }
        void sync();
        void setCleanStart(bool value);
        void setMaxInflightMessages();
        void setCredentials(EncodedString name, EncodedString password);
        bool connected();

        void setHandler(MqttClientHandler *hander);

        /* Utilities */
        /**
         * @brief Whether a token has been delivered
         *
         * @param token
         * @return true
         * @return false
         */
        bool isDelivered(uint16_t token);

        void setClientId(EncodedString &id);
        void setClientId(const char *data, uint16_t length);

        /* Client properties Getters/Setters */
        uint16_t getKeepAliveInterval();
        void setKeepAliveInterval(uint16_t value);
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

        void setAutoReconnect(int32_t value);
        int getAutoReconnect();

        /* Publish Actions */
        /**
         * @brief Publish a payload over MQTT
         *
         * @param topic The topic to publish the payload with
         * @param payload The payload to publish
         * @param qos The QOS of the payload to publish
         * @return uint16_t The unique token used to identify a publish packet, or
         * PUBLISH_FAILED (0) if the packet could not be sent. 0 is never a valid token.
         */
        uint16_t publish(EncodedString &topic, Payload &payload, QoS qos, bool retain = false);

        /* Subscribe Actions */
    };
}

#endif /* SRC_MQTTCLIENT */
