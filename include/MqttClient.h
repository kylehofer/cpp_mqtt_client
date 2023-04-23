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

#define IDLE 0x0
#define CLIENT_CONNECTING 0x1
#define SSL_IN_PROGRESS 0x2
#define WAIT_FOR_CONNACK 0x4
#define DISCONNECTED -2

// typedef struct
// {
//     /** The eyecatcher for this structure.  must be MQTC. */
//     char struct_id[4];
//     /** The version number of this structure.  Must be 0, 1, 2, 3, 4, 5, 6, 7 or 8.
//      * 0 signifies no SSL options and no serverURIs
//      * 1 signifies no serverURIs
//      * 2 signifies no MQTTVersion
//      * 3 signifies no returned values
//      * 4 signifies no binary password option
//      * 5 signifies no maxInflightMessages and cleanstart
//      * 6 signifies no HTTP headers option
//      * 7 signifies no HTTP proxy and HTTPS proxy options
//      */
//     int struct_version;
//     /** The "keep alive" interval, measured in seconds, defines the maximum time
//      * that should pass without communication between the client and the server
//      * The client will ensure that at least one message travels across the
//      * network within each keep alive period.  In the absence of a data-related
//      * message during the time period, the client sends a very small MQTT
//      * "ping" message, which the server will acknowledge. The keep alive
//      * interval enables the client to detect when the server is no longer
//      * available without having to wait for the long TCP/IP timeout.
//      */
//     int keepAliveInterval;
//     /**
//      * This is a boolean value. The cleansession setting controls the behaviour
//      * of both the client and the server at connection and disconnection time.
//      * The client and server both maintain session state information. This
//      * information is used to ensure "at least once" and "exactly once"
//      * delivery, and "exactly once" receipt of messages. Session state also
//      * includes subscriptions created by an MQTT client. You can choose to
//      * maintain or discard state information between sessions.
//      *
//      * When cleansession is true, the state information is discarded at
//      * connect and disconnect. Setting cleansession to false keeps the state
//      * information. When you connect an MQTT client application with
//      * MQTTClient_connect(), the client identifies the connection using the
//      * client identifier and the address of the server. The server checks
//      * whether session information for this client
//      * has been saved from a previous connection to the server. If a previous
//      * session still exists, and cleansession=true, then the previous session
//      * information at the client and server is cleared. If cleansession=false,
//      * the previous session is resumed. If no previous session exists, a new    uint32_t sessionExpiryInterval;
// uint16_t receiveMaximum;
// uint32_t maximumPacketSize;
// uint16_t topicAliasMaximum;
//      * session is started.
//      */
//     int cleansession;
//     /**
//      * This is a boolean value that controls how many messages can be in-flight
//      * simultaneously. Setting <i>reliable</i> to true means that a published
//      * message must be completed (acknowledgements received) before another
//      * can be sent. Attempts to publish additional messages receive an
//      * ::MQTTCLIENT_MAX_MESSAGES_INFLIGHT return code. Setting this flag to
//      * false allows up to 10 messages to be in-flight. This can increase
//      * overall throughput in some circumstances.
//      */
//     int reliable;
//     /**
//      * MQTT servers that support the MQTT v3.1.1 protocol provide authentication
//      * and authorisation by user name and password. This is the user name
//      * parameter.
//      */
//     const char *username;
//     /**
//      * MQTT servers that support the MQTT v3.1.1 protocol provide authentication
//      * and authorisation by user name and password. This is the password
//      * parameter.
//      */
//     const char *password;
//     /**
//      * The time interval in seconds to allow a connect to complete.
//      */
//     int connectTimeout;
//     /**
//      * The time interval in seconds after which unacknowledged publish requests are
//      * retried during a TCP session.  With MQTT 3.1.1 and later, retries are
//      * not required except on reconnect.  0 turns off in-session retries, and is the
//      * recommended setting.  Adding retries to an already overloaded network only
//      * exacerbates the problem.
//      */
//     int retryInterval;
//     /**
//      * This is a pointer to an MQTTClient_SSLOptions structure. If your
//      * application does not make use of SSL, set this pointer to NULL.
//      */
//     MQTTClient_SSLOptions *ssl;
//     /**
//      * The number of entries in the optional serverURIs array. Defaults to 0.
//      */
//     int serverURIcount;
//     /**
//      * An optional array of null-terminated strings specifying the servers to
//      * which the client will connect. Each string takes the form <i>protocol://host:port</i>.
//      * <i>protocol</i> must be <i>tcp</i>, <i>ssl</i>, <i>ws</i> or <i>wss</i>.
//      * The TLS enabled prefixes (ssl, wss) are only valid if a TLS version of the library
//      * is linked with.
//      * For <i>host</i>, you can
//      * specify either an IP address or a host name. For instance, to connect to
//      * a server running on the local machines with the default MQTT port, specify
//      * <i>tcp://localhost:1883</i>.
//      * If this list is empty (the default), the server URI specified on MQTTClient_create()
//      * is used.
//      */
//     char *const *serverURIs;
//     /**
//      * Sets the version of MQTT to be used on the connect.
//      * MQTTVERSION_DEFAULT (0) = default: start with 3.1.1, and if that fails, fall back to 3.1
//      * MQTTVERSION_3_1 (3) = only try version 3.1
//      * MQTTVERSION_3_1_1 (4) = only try version 3.1.1
//      * MQTTVERSION_5 (5) = only try version 5.0
//      */
//     int MQTTVersion;
//     /**
//      * Returned from the connect when the MQTT version used to connect is 3.1.1
//      */
//     struct
//     {
//         const char *serverURI; /**< the serverURI connected to */
//         int MQTTVersion;       /**< the MQTT version used to connect with */
//         int sessionPresent;    /**< if the MQTT version is 3.1.1, the value of sessionPresent returned in the connack */
//     } returned;
//     /**
//      * Optional binary password.  Only checked and used if the password option is NULL
//      */
//     struct
//     {
//         int len;          /**< binary password length */
//         const void *data; /**< binary password data */
//     } binarypwd;
//     /**
//      * The maximum number of messages in flight
//      */
//     int maxInflightMessages;
//     /*
//      * MQTT V5 clean start flag.  Only clears state at the beginning of the session.
//      */
//     int cleanstart;
//     /**
//      * HTTP headers for websockets
//      */
//     const MQTTClient_nameValue *httpHeaders;
//     /**
//      * HTTP proxy
//      */
//     const char *httpProxy;
//     /**
//      * HTTPS proxy
//      */
//     const char *httpsProxy;

//     const char *address;
//     int port;
//     const char *uri;
//     int connectTimeout;
//     bool reliable;

// } ConnectOptions;

#include "properties/WillProperties.h"
#include "types/SubscriptionPayload.h"
#include "types/Payload.h"
#include "types/EncodedString.h"
#include "packets/Connect.h"
#include "packets/Packet.h"
#include "packets/ConnectAcknowledge.h"
#include "packets/Disconnect.h"
#include <tuple>
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

        bool hasClientToken(uint16_t token);
        void removeClientToken(uint16_t token);

    protected:
        uint32_t getElapsed();

    public:
        MqttClient();
        template <typename CommunicationClient>
        MqttClient();
        MqttClient(Client *client);
        int setWill(WillProperties *);
        int connect(const char *address, int port, uint32_t connectTimeout);
        int disconnect();
        int subscribe(SubscriptionPayload &payload...);
        int unsubscribe(SubscriptionPayload &payload...);
        void sync();
        void setCleanStart(bool value);
        void setMaxInflightMessages();
        void setCredentials(EncodedString name, EncodedString password);
        bool connected();

        /* Utilities */
        uint16_t getPacketIdentifier();
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
        uint16_t publish(EncodedString &topic, Payload &payload, QoS qos);

        /* Subscribe Actions */
    };
}

#endif /* MQTTCLIENT */
