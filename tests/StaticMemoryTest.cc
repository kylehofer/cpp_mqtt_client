/*
 * File: StaticMemoryTest.cc
 * Project: cpp_mqtt_client
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
 */

/**
 * Verifies that a STATIC_MEMORY build never touches the heap, by counting
 * allocations through a whole client session.
 */

#include "gtest/gtest.h"
#include "mocks/FixedBufferClient.h"
#include "support/AllocationCounter.h"
#include "MqttClient.h"

using namespace CppMqtt;

namespace
{
    // Records results without allocating, so it can run while counting
    class SessionHandler : public MqttClientHandler
    {
    public:
        int connected = 0;
        int messages = 0;
        int deliveries = 0;
        int failures = 0;
        int subscribeResults = 0;
        size_t subscribeCodes = 0;
        int disconnections = 0;
        uint8_t lastPayload[16] = {0};
        size_t lastPayloadSize = 0;

        void onConnectionSuccess() override { connected++; }
        void onConnectionFailure(int) override { failures++; }
        void onDisconnection(ReasonCode) override { disconnections++; }
        void onMessage(EncodedString &, Payload &payload) override
        {
            messages++;
            lastPayloadSize = payload.size() < sizeof(lastPayload) ? payload.size() : sizeof(lastPayload);
            memcpy(lastPayload, payload.getData(), lastPayloadSize);
        }
        void onDeliveryComplete(Token) override { deliveries++; }
        void onDeliveryFailure(Token, int) override { failures++; }
        void onSubscribeResult(Token, ReasonCodes reasonCodes) override
        {
            subscribeResults++;
            subscribeCodes = reasonCodes.size();
        }
        void onUnsubscribeResult(Token, ReasonCodes) override {}
    };

    void feed(FixedBufferClient &client, MqttClient &mqttClient, const unsigned char *data, size_t size)
    {
        client.feed(data, size);
        mqttClient.sync();
    }
}

/**
 * Runs connect, a two topic subscribe, publishes at every QoS with their
 * acknowledgements, incoming QoS 1 and QoS 2 messages with properties, a ping
 * response, an unknown packet and a disconnect.
 *
 * @return size_t Heap allocations made during the session
 */
static size_t runSession(SessionHandler &handler, bool &connectedOk, bool &disconnectedOk)
{
    FixedBufferClient client;

    AllocationCounter::start();
    {
        MqttClient mqttClient(&client);
        mqttClient.setHandler(&handler);
        mqttClient.setClientId("device-1", 8);
        mqttClient.connect("broker.local", 1883, 1000);
        mqttClient.sync(); // sends CONNECT

        const unsigned char connack[] = {
            0x20, 0x12, 0x00, 0x00, 0x0F,              // CONNACK, properties length 15
            0x13, 0x00, 0x3C,                          // Server Keep Alive 60
            0x1F, 0x00, 0x02, 'o', 'k',                // Reason String
            0x26, 0x00, 0x01, 'k', 0x00, 0x01, 'v'};   // User Property
        feed(client, mqttClient, connack, sizeof(connack));
        connectedOk = mqttClient.connected();

        SubscribePayload first, second;
        first.setTopic("a/1", 3);
        second.setTopic("a/2", 3);
        Token subscribeToken = mqttClient.subscribe(first, second);
        const unsigned char suback[] = {0x90, 0x05, (uint8_t)(subscribeToken >> 8), (uint8_t)subscribeToken, 0x00, 0x00, 0x01};
        feed(client, mqttClient, suback, sizeof(suback));

        uint8_t data[] = {'d', 'a', 't', 'a'};
        Payload payload = Payload::wrap(data, sizeof(data));
        EncodedString topic("t/out", 5);

        mqttClient.publish(topic, payload, QoS::ZERO);
        mqttClient.sync();

        Token one = mqttClient.publish(topic, payload, QoS::ONE);
        const unsigned char puback[] = {0x40, 0x03, (uint8_t)(one >> 8), (uint8_t)one, 0x00};
        feed(client, mqttClient, puback, sizeof(puback));

        Token two = mqttClient.publish(topic, payload, QoS::TWO);
        const unsigned char pubrec[] = {0x50, 0x03, (uint8_t)(two >> 8), (uint8_t)two, 0x00};
        const unsigned char pubcomp[] = {0x70, 0x03, (uint8_t)(two >> 8), (uint8_t)two, 0x00};
        feed(client, mqttClient, pubrec, sizeof(pubrec));
        feed(client, mqttClient, pubcomp, sizeof(pubcomp));

        const unsigned char incomingQos1[] = {
            0x32, 0x19,                              // PUBLISH QoS 1, remaining length 25
            0x00, 0x03, 't', '/', 'a',               // Topic
            0x00, 0x07,                              // Packet Identifier
            0x0C,                                    // Properties length 12
            0x02, 0x00, 0x00, 0x00, 0x3C,            // Message Expiry Interval
            0x26, 0x00, 0x01, 'k', 0x00, 0x01, 'v', // User Property
            'h', 'e', 'l', 'l', 'o'};                // Payload
        feed(client, mqttClient, incomingQos1, sizeof(incomingQos1));

        const unsigned char incomingQos2[] = {0x34, 0x09, 0x00, 0x03, 't', '/', 'b', 0x00, 0x08, 0x00, 'x'};
        const unsigned char pubrel[] = {0x62, 0x03, 0x00, 0x08, 0x00};
        feed(client, mqttClient, incomingQos2, sizeof(incomingQos2));
        feed(client, mqttClient, pubrel, sizeof(pubrel));

        const unsigned char pingresp[] = {0xD0, 0x00};
        const unsigned char unknown[] = {0x00, 0x01, 0xAA};
        feed(client, mqttClient, pingresp, sizeof(pingresp));
        feed(client, mqttClient, unknown, sizeof(unknown));

        disconnectedOk = mqttClient.disconnect(ReasonCode::NORMAL_DISCONNECTION) == 0;
    }
    return AllocationCounter::stop();
}

TEST(StaticMemoryTest, FullSessionDoesNotAllocate)
{
    if (!AllocationCounter::available())
    {
        GTEST_SKIP() << "Allocation counting needs glibc and no sanitizer";
    }

    SessionHandler handler;
    bool connectedOk = false;
    bool disconnectedOk = false;
    size_t allocations = runSession(handler, connectedOk, disconnectedOk);

    // The session must have actually run for the count to mean anything
    EXPECT_TRUE(connectedOk);
    EXPECT_TRUE(disconnectedOk);
    EXPECT_EQ(handler.connected, 1);
    EXPECT_EQ(handler.subscribeResults, 1);
    EXPECT_EQ(handler.subscribeCodes, (size_t)2);
    EXPECT_EQ(handler.deliveries, 3); // QoS 0, 1 and 2
    EXPECT_EQ(handler.failures, 0);
    EXPECT_EQ(handler.messages, 2);   // incoming QoS 1 and QoS 2
    EXPECT_EQ(handler.disconnections, 1);

#ifdef STATIC_MEMORY
    EXPECT_EQ(allocations, (size_t)0) << "STATIC_MEMORY build allocated on the heap";
#else
    // Dynamic memory is expected to allocate; recorded for comparison
    RecordProperty("allocations", (int)allocations);
    EXPECT_GT(allocations, (size_t)0);
#endif
}

TEST(StaticMemoryTest, ReceivedPayloadIsDelivered)
{
    SessionHandler handler;
    bool connectedOk = false;
    bool disconnectedOk = false;
    runSession(handler, connectedOk, disconnectedOk);

    // The last message delivered is the QoS 2 one with payload "x"
    ASSERT_EQ(handler.lastPayloadSize, (size_t)1);
    EXPECT_EQ(handler.lastPayload[0], 'x');
}

#ifdef STATIC_MEMORY
TEST(StaticMemoryTest, ConnectAnnouncesLimits)
{
    FixedBufferClient client;
    MqttClient mqttClient(&client);
    mqttClient.connect("broker.local", 1883, 1000);
    mqttClient.sync();

    const uint8_t *data = client.writtenData();
    ASSERT_GE(client.written(), (size_t)21);
    // Fixed header (2), protocol (7), flags (1), keep alive (2), then properties
    EXPECT_EQ(data[12], 8) << "properties length";
    EXPECT_EQ(data[13], 0x21) << "Receive Maximum";
    EXPECT_EQ((data[14] << 8) | data[15], CPP_MQTT_MAX_INFLIGHT);
    EXPECT_EQ(data[16], 0x27) << "Maximum Packet Size";
    EXPECT_EQ(((uint32_t)data[17] << 24) | (data[18] << 16) | (data[19] << 8) | data[20], (uint32_t)CPP_MQTT_MAX_PAYLOAD_LENGTH);
}

TEST(StaticMemoryTest, OversizedPayloadIsTruncatedAndStreamStaysAligned)
{
    FixedBufferClient client;
    PacketReader reader;

    const size_t payloadSize = CPP_MQTT_MAX_PAYLOAD_LENGTH + 10;
    const size_t remaining = 3 + 1 + payloadSize; // topic "a", properties length, payload
    unsigned char header[] = {0x30, (uint8_t)(0x80 | (remaining & 0x7F)), (uint8_t)(remaining >> 7), 0x00, 0x01, 'a', 0x00};
    client.feed(header, sizeof(header));
    unsigned char chunk[64];
    memset(chunk, 'z', sizeof(chunk));
    for (size_t sent = 0; sent < payloadSize; sent += sizeof(chunk))
    {
        client.feed(chunk, (payloadSize - sent) < sizeof(chunk) ? (payloadSize - sent) : sizeof(chunk));
    }
    const unsigned char pingresp[] = {0xD0, 0x00};
    client.feed(pingresp, sizeof(pingresp));

    Packet *packet = reader.read(&client);
    ASSERT_NE(packet, nullptr);
    Publish *publish = (Publish *)packet;
    EXPECT_TRUE(publish->getPayload().wasTruncated());
    EXPECT_EQ(publish->getPayload().size(), (size_t)CPP_MQTT_MAX_PAYLOAD_LENGTH);
    releasePacket(packet);

    packet = reader.read(&client);
    ASSERT_NE(packet, nullptr);
    EXPECT_EQ(packet->getPacketType(), PacketId::PING_RESPONSE);
    releasePacket(packet);
}

TEST(StaticMemoryTest, PropertyPoolExhaustionIsHandled)
{
    Properties properties;
    size_t added = 0;
    while (properties.add<UserProperty>(EncodedString("k", 1), EncodedString("v", 1)) != NULL)
    {
        added++;
    }
    // Stops at whichever limit is smaller, without crashing
    size_t limit = CPP_MQTT_MAX_PROPERTIES < CPP_MQTT_PROPERTY_POOL_SIZE ? CPP_MQTT_MAX_PROPERTIES : CPP_MQTT_PROPERTY_POOL_SIZE;
    EXPECT_EQ(added, limit);
    EXPECT_EQ(properties.length(), (uint32_t)limit);

    // Clearing returns every slot to the pool
    size_t availableBefore = propertyPoolAvailable();
    properties.clear();
    EXPECT_EQ(propertyPoolAvailable(), availableBefore + limit);
}
#endif
