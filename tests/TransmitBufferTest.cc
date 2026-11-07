/*
 * File: TransmitBufferTest.cc
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
 * Tests for encoding packets straight into a network client's transmit buffer.
 */

#include <vector>
#include "gtest/gtest.h"
#include "mocks/MockClient.h"
#include "utils/TestPacketBuffer.h"
#include "mocks/RingTransportClient.h"
#include "support/AllocationCounter.h"
#include "utils/TransmitRing.h"
#include "MqttClient.h"

using namespace CppMqtt;

namespace
{
    template <size_t N>
    void connectThroughRing(RingTransportClient<N> &client, MqttClient &mqttClient)
    {
        mqttClient.connect("broker.local", 1883, 1000);
        mqttClient.sync(); // CONNECT goes into the ring
        client.deliverAndAcknowledge();
        const unsigned char connack[] = {0x20, 0x03, 0x00, 0x00, 0x00};
        client.feed(connack, sizeof(connack));
        mqttClient.sync();
        client.clearWire();
    }

    std::vector<uint8_t> publishThroughWrite(EncodedString &topic, Payload &payload, QoS qos)
    {
        MockClient client;
        MqttClient mqttClient((Client *)&client);
        client.setIsConnected(true);
        mqttClient.connect("broker.local", 1883, 1000);
        mqttClient.sync();
        const unsigned char connack[] = {0x20, 0x03, 0x00, 0x00, 0x00};
        client.pushToReadBuffer((void *)connack, sizeof(connack));
        mqttClient.sync();
        client.clearWriteBuffer();

        mqttClient.publish(topic, payload, qos);
        const uint8_t *data = (const uint8_t *)client.getWriteBuffer();
        return std::vector<uint8_t>(data, data + client.written());
    }
}

TEST(TransmitRingTest, WritesWrapAcrossTheEnd)
{
    TransmitRing<8> ring;
    size_t contiguous = 0;

    // Fill 6 bytes, send and acknowledge them so the write position is near the end
    ASSERT_NE(ring.writePointer(0, contiguous), nullptr);
    EXPECT_EQ(contiguous, (size_t)8);
    ASSERT_TRUE(ring.commit(6));
    size_t length = 0;
    ring.unsent(length);
    ring.markSent(length);
    ring.release(6);
    EXPECT_EQ(ring.writable(), (size_t)8);

    // The next write is split by the wrap: 2 bytes at the end, then 6 at the start
    uint8_t *first = ring.writePointer(0, contiguous);
    EXPECT_EQ(contiguous, (size_t)2);
    uint8_t *second = ring.writePointer(2, contiguous);
    EXPECT_EQ(contiguous, (size_t)6);
    EXPECT_LT(second, first);
    memcpy(first, "ab", 2);
    memcpy(second, "cdefgh", 6);
    ASSERT_TRUE(ring.commit(8));
    EXPECT_EQ(ring.writable(), (size_t)0);
    EXPECT_EQ(ring.writePointer(0, contiguous), nullptr);

    // Unsent data comes back in two spans, in order
    const uint8_t *span = ring.unsent(length);
    ASSERT_EQ(length, (size_t)2);
    EXPECT_EQ(memcmp(span, "ab", 2), 0);
    ring.markSent(length);
    span = ring.unsent(length);
    ASSERT_EQ(length, (size_t)6);
    EXPECT_EQ(memcmp(span, "cdefgh", 6), 0);
    ring.markSent(length);

    // Space returns only as data is acknowledged
    EXPECT_EQ(ring.writable(), (size_t)0);
    ring.release(3);
    EXPECT_EQ(ring.writable(), (size_t)3);
    EXPECT_FALSE(ring.commit(4));
}

TEST(TransmitBufferTest, PublishMatchesTheWritePath)
{
    EncodedString topic("sensors/temperature", 19);
    uint8_t data[40];
    for (size_t i = 0; i < sizeof(data); i++)
    {
        data[i] = (uint8_t)i;
    }
    Payload payload = Payload::wrap(data, sizeof(data));
    std::vector<uint8_t> expected = publishThroughWrite(topic, payload, QoS::ONE);
    ASSERT_GT(expected.size(), (size_t)0);

    RingTransportClient<256> client;
    MqttClient mqttClient(&client, &client);
    connectThroughRing(client, mqttClient);
    int writesBefore = client.writes;

    Token token = mqttClient.publish(topic, payload, QoS::ONE);
    ASSERT_NE(token, PUBLISH_FAILED);
    EXPECT_EQ(client.writes, writesBefore) << "a transmit buffer must not fall back to write()";
    client.deliver();

    ASSERT_EQ(client.wireLength(), expected.size());
    EXPECT_EQ(memcmp(client.wireData(), expected.data(), expected.size()), 0);
}

TEST(TransmitBufferTest, PacketSplitAcrossTheWrapIsIntact)
{
    EncodedString topic("t", 1);
    uint8_t data[30];
    memset(data, 0x5A, sizeof(data));
    Payload payload = Payload::wrap(data, sizeof(data));
    std::vector<uint8_t> expected = publishThroughWrite(topic, payload, QoS::ZERO);

    // 64 byte ring: after a few publishes the write position wraps mid packet
    RingTransportClient<64 + CPP_MQTT_TRANSMIT_RESERVE> client;
    MqttClient mqttClient(&client, &client);
    connectThroughRing(client, mqttClient);

    for (int i = 0; i < 5; i++)
    {
        client.clearWire();
        ASSERT_NE(mqttClient.publish(topic, payload, QoS::ZERO), PUBLISH_FAILED) << "publish " << i;
        client.deliverAndAcknowledge();
        mqttClient.sync(); // reports the QoS 0 delivery, emptying the result list
        ASSERT_EQ(client.wireLength(), expected.size()) << "publish " << i;
        EXPECT_EQ(memcmp(client.wireData(), expected.data(), expected.size()), 0) << "publish " << i;
    }
}

TEST(TransmitBufferTest, FullBufferRefusesPublishWithoutDisconnecting)
{
    RingTransportClient<64 + CPP_MQTT_TRANSMIT_RESERVE> client;
    MqttClient mqttClient(&client, &client);
    connectThroughRing(client, mqttClient);

    EncodedString topic("t", 1);
    uint8_t data[50];
    Payload payload = Payload::wrap(data, sizeof(data));

    ASSERT_NE(mqttClient.publish(topic, payload, QoS::ONE), PUBLISH_FAILED);
    // Not yet acknowledged by the network, so a second one does not fit
    EXPECT_EQ(mqttClient.publish(topic, payload, QoS::ONE), PUBLISH_FAILED);
    EXPECT_TRUE(mqttClient.connected());
    EXPECT_TRUE(client.connected());

    // Once the network acknowledges the first, the next one fits
    client.deliverAndAcknowledge();
    EXPECT_NE(mqttClient.publish(topic, payload, QoS::ONE), PUBLISH_FAILED);
}

TEST(TransmitBufferTest, ReserveLeavesRoomForAcknowledgements)
{
    RingTransportClient<64 + CPP_MQTT_TRANSMIT_RESERVE> client;
    MqttClient mqttClient(&client, &client);
    connectThroughRing(client, mqttClient);

    // Fill the buffer with the application's publishes until one is refused
    EncodedString topic("t", 1);
    uint8_t data[8];
    Payload payload = Payload::wrap(data, sizeof(data));
    int published = 0;
    while (mqttClient.publish(topic, payload, QoS::ONE) != PUBLISH_FAILED)
    {
        published++;
    }
    ASSERT_GT(published, 0);
    EXPECT_GE(client.writable(), (size_t)CPP_MQTT_TRANSMIT_RESERVE) << "the reserve must stay free";

    // An incoming QoS 1 message must still be acknowledged
    client.deliver();
    client.clearWire();
    const unsigned char incoming[] = {0x32, 0x07, 0x00, 0x01, 'x', 0x00, 0x09, 0x00, '!'};
    client.feed(incoming, sizeof(incoming));
    mqttClient.sync();
    client.deliver();

    PublishAcknowledge expected;
    expected.setPacketIdentifier(9);
    expected.setReasonCode(0);
    TestPacketBuffer encoded;
    expected.push(encoded);
    ASSERT_EQ(client.wireLength(), encoded.getLength());
    EXPECT_EQ(memcmp(client.wireData(), encoded.getBuffer(), encoded.getLength()), 0);
}

TEST(TransmitBufferTest, PacketsAreCommittedWhole)
{
    RingTransportClient<256> client;
    MqttClient mqttClient(&client, &client);
    connectThroughRing(client, mqttClient);
    int commitsBefore = client.commits;

    EncodedString topic("a/b", 3);
    uint8_t data[100];
    Payload payload = Payload::wrap(data, sizeof(data));
    ASSERT_NE(mqttClient.publish(topic, payload, QoS::ZERO), PUBLISH_FAILED);

    // One commit for the whole packet, however many fields it was encoded from
    EXPECT_EQ(client.commits, commitsBefore + 1);
}

TEST(TransmitBufferTest, PublishDoesNotAllocate)
{
    if (!AllocationCounter::available())
    {
        GTEST_SKIP() << "Allocation counting needs glibc and no sanitizer";
    }

    RingTransportClient<512> client;
    MqttClient mqttClient(&client, &client);
    connectThroughRing(client, mqttClient);

    EncodedString topic("sensors/1", 9);
    uint8_t data[200];
    Payload payload = Payload::wrap(data, sizeof(data));

    AllocationCounter::start();
    Token token = mqttClient.publish(topic, payload, QoS::ONE);
    size_t allocations = AllocationCounter::stop();

    ASSERT_NE(token, PUBLISH_FAILED);
#ifdef STATIC_MEMORY
    EXPECT_EQ(allocations, (size_t)0);
#else
    // Dynamic mode copies the topic into the packet and grows the in-flight
    // token list; the payload and the encoded packet never touch the heap
    EXPECT_LE(allocations, (size_t)2);
#endif
}
