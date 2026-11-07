/*
 * File: RegressionTests.cc
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
 * Regression tests for wire-format and robustness bugs found in review.
 */

#include "gtest/gtest.h"
#include "utils/TestPacketBuffer.h"
#include "stdint.h"
#include "mocks/MockClient.h"
#include "mocks/FixedBufferClient.h"
#include "mocks/MockMqttClient.h"
#include "MqttProperties.h"
#include "MqttClient.h"
#include "packets/PacketUtility.h"
#include "properties/WillProperties.h"

using namespace std;
using namespace CppMqtt;

// Defined in MqttClientTests.cc
void setupConnected(MockClient &client, MqttClient &mqttClient);

namespace
{
    class CountingHandler : public MqttClientHandler
    {
    public:
        int messages = 0;
        void onConnectionSuccess() override {}
        void onConnectionFailure(int) override {}
        void onDisconnection(ReasonCode) override {}
        void onMessage(EncodedString &, Payload &) override { messages++; }
        void onDeliveryComplete(Token) override {}
        void onDeliveryFailure(Token, int) override {}
        void onSubscribeResult(Token, ReasonCodes) override {}
        void onUnsubscribeResult(Token, ReasonCodes) override {}
    };
}

TEST(RegressionTests, ConnectKeepAliveIsBigEndianSixteenBit)
{
    Connect connect;
    connect.setKeepAliveInterval(300); // 0x012C
    TestPacketBuffer buffer;
    connect.push(buffer);
    uint8_t *data = buffer.getBuffer();

    // Fixed header (2) + protocol name and version (7) + connect flags (1)
    ASSERT_EQ(data[9], 0x00) << "connect flags";
    ASSERT_EQ(data[10], 0x01) << "keep alive MSB";
    ASSERT_EQ(data[11], 0x2C) << "keep alive LSB";
    ASSERT_EQ(connect.getKeepAliveInterval(), 300);
}

TEST(RegressionTests, PublishPacketIdentifierIsBigEndian)
{
    Publish publish;
    publish.setTopic("t", 1);
    publish.setQos(QoS::ONE);
    publish.setPacketIdentifier(0x0102);
    TestPacketBuffer buffer;
    publish.push(buffer);
    uint8_t *data = buffer.getBuffer();

    // Fixed header (2) + topic length (2) + topic (1)
    ASSERT_EQ(data[5], 0x01);
    ASSERT_EQ(data[6], 0x02);
}

TEST(RegressionTests, AcknowledgePacketIdentifierReadAsBigEndian)
{
    MockClient client;
    const unsigned char puback[] = {0x40, 0x03, 0x01, 0x02, 0x00};
    client.pushToReadBuffer((void *)puback, sizeof(puback));

    Packet *packet = readPacketFromClient((Client *)&client);
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(packet->getPacketType(), PacketId::PUBLISH_ACKNOWLEDGE);
    ASSERT_EQ(((PublishAcknowledge *)packet)->getPacketIdentifier(), 0x0102);
    releasePacket(packet);
}

TEST(RegressionTests, BufferDataEquality)
{
    EncodedString a("abc", 3), same("abc", 3), different("xyz", 3), longer("abcd", 4);
    EncodedString emptyA, emptyB;

    ASSERT_TRUE(a == same);
    ASSERT_FALSE(a != same);
    ASSERT_FALSE(a == different);
    ASSERT_FALSE(a == longer);
    ASSERT_TRUE(emptyA == emptyB);
}

TEST(RegressionTests, PropertiesEachVisitsEveryMatchOnce)
{
    Properties properties;
    ASSERT_NE(properties.add<UserProperty>(EncodedString("k1", 2), EncodedString("v1", 2)), nullptr);
    ASSERT_NE(properties.add<UserProperty>(EncodedString("k2", 2), EncodedString("v2", 2)), nullptr);
    ASSERT_NE(properties.add<UserProperty>(EncodedString("k3", 2), EncodedString("v3", 2)), nullptr);

    int visited = 0;
    properties.each<UserProperty>(USER_PROPERTY, [&visited](UserProperty *)
                                  { visited++; return true; });
    ASSERT_EQ(visited, 3);

    // Stopping early on the second match
    EncodedString wanted("k2", 2);
    EncodedString found;
    visited = 0;
    properties.each<UserProperty>(USER_PROPERTY, [&](UserProperty *property)
                                  {
                                      visited++;
                                      if (property->getKey() == wanted)
                                      {
                                          found = property->getValue();
                                          return false;
                                      }
                                      return true; });
    ASSERT_EQ(visited, 2);
    ASSERT_TRUE(found == EncodedString("v2", 2));
}

TEST(RegressionTests, WillUtf8PayloadIsStored)
{
    WillProperties will;
    will.setWillTopic("t", 1);
    EncodedString payload("hello", 5);
    will.setUtf8Payload(payload);

    // Properties: length (1) + payload format indicator (2)
    // Topic: length (2) + 1, Payload: length (2) + 5
    ASSERT_EQ(will.size(), (size_t)13);
}

TEST(RegressionTests, WillBinaryPayloadIsStored)
{
    WillProperties will;
    will.setWillTopic("t", 1);
    BinaryData payload("\x01\x02\x03", 3);
    will.setBinaryPayload(payload);

    ASSERT_EQ(will.size(), (size_t)11);
}

TEST(RegressionTests, UnknownPacketTypeIsSkipped)
{
    MockClient client;
    // Packet type 0 is reserved, followed by two bytes of contents, then a PINGRESP
    const unsigned char stream[] = {0x00, 0x02, 0xAA, 0xBB, 0xD0, 0x00};
    client.pushToReadBuffer((void *)stream, sizeof(stream));

    Packet *packet = readPacketFromClient((Client *)&client);
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(packet->getPacketType(), PacketId::PING_RESPONSE);
    ASSERT_EQ(client.available(), 0);
    releasePacket(packet);
}

TEST(RegressionTests, UnknownPropertyIsSkipped)
{
    MockClient client;
    const unsigned char stream[] = {
        0x40, 0x07,       // PUBACK, remaining length 7
        0x00, 0x05,       // Packet Identifier
        0x00,             // Reason Code
        0x03,             // Properties length
        0x7F, 0xAA, 0xBB, // Unknown property identifier and data
        0xD0, 0x00        // PINGRESP, to check the stream stays aligned
    };
    client.pushToReadBuffer((void *)stream, sizeof(stream));

    Packet *packet = readPacketFromClient((Client *)&client);
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(packet->getPacketType(), PacketId::PUBLISH_ACKNOWLEDGE);
    ASSERT_EQ(((PublishAcknowledge *)packet)->getPacketIdentifier(), 0x0005);
    releasePacket(packet);

    packet = readPacketFromClient((Client *)&client);
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(packet->getPacketType(), PacketId::PING_RESPONSE);
    releasePacket(packet);
}

TEST(RegressionTests, InboundQos2DeliveredOnceUntilRelease)
{
    MockClient client;
    MqttClient mqttClient((Client *)&client);
    CountingHandler handler;
    mqttClient.setHandler(&handler);
    setupConnected(client, mqttClient);

    const unsigned char publish[] = {
        0x34, 0x09,       // PUBLISH QoS 2, remaining length 9
        0x00, 0x01, 'a',  // Topic
        0x00, 0x05,       // Packet Identifier
        0x03,             // Properties length
        0x23, 0x00, 0x01, // Topic Alias property, so the packet carries a property
    };
    // Remaining length: topic (3) + identifier (2) + properties (1 + 3) = 9, no payload
    const unsigned char pubrel[] = {0x62, 0x03, 0x00, 0x05, 0x00};

    client.pushToReadBuffer((void *)publish, sizeof(publish));
    mqttClient.sync();
    ASSERT_EQ(handler.messages, 1);

    // A retransmission before PUBREL must not be delivered again
    client.pushToReadBuffer((void *)publish, sizeof(publish));
    mqttClient.sync();
    ASSERT_EQ(handler.messages, 1);

    client.pushToReadBuffer((void *)pubrel, sizeof(pubrel));
    mqttClient.sync();
    ASSERT_EQ(handler.messages, 1);

    // After release the identifier may be reused for a new message
    client.pushToReadBuffer((void *)publish, sizeof(publish));
    mqttClient.sync();
    ASSERT_EQ(handler.messages, 2);
}

namespace
{
    /**
     * Delivers a packet one byte at a time, as a slow TCP link might.
     * Returns the parsed packet, or NULL if one was returned early or never.
     */
    Packet *readOneByteAtATime(MockClient &client, const unsigned char *data, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            client.pushToReadBuffer((void *)&data[i], 1);
            Packet *packet = readPacketFromClient((Client *)&client);
            if (i + 1 < size && packet != NULL)
            {
                ADD_FAILURE() << "packet returned early at byte " << i;
                releasePacket(packet);
                return NULL;
            }
            if (i + 1 == size)
            {
                return packet;
            }
        }
        return NULL;
    }
}

// Bug: a packet split inside a multi-byte field (identifier, string length,
// property value) made the parsers spin forever waiting inside sync().
TEST(RegressionTests, AcknowledgeSplitAcrossReads)
{
    MockClient client;
    const unsigned char puback[] = {
        0x40, 0x08,                  // PUBACK, remaining length 8
        0x01, 0x02,                  // Packet Identifier
        0x10,                        // Reason Code: No matching subscribers
        0x04,                        // Properties length
        0x1F, 0x00, 0x01, 'x',       // Reason String "x"
    };
    Packet *packet = readOneByteAtATime(client, puback, sizeof(puback));
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(((PublishAcknowledge *)packet)->getPacketIdentifier(), 0x0102);
    ASSERT_EQ(((PublishAcknowledge *)packet)->getReasonCode(), 0x10);
    releasePacket(packet);
}

TEST(RegressionTests, PublishSplitAcrossReads)
{
    MockClient client;
    const unsigned char publish[] = {
        0x32, 0x0D,                   // PUBLISH QoS 1, remaining length 13
        0x00, 0x01, 'a',              // Topic
        0x00, 0x07,                   // Packet Identifier
        0x05,                         // Properties length
        0x02, 0x00, 0x00, 0x00, 0x3C, // Message Expiry Interval 60
        'h', 'i'                      // Payload
    };
    Packet *packet = readOneByteAtATime(client, publish, sizeof(publish));
    ASSERT_NE(packet, nullptr);
    Publish *received = (Publish *)packet;
    ASSERT_EQ(received->getPacketIdentifier(), 7);
    ASSERT_TRUE(received->getTopic() == EncodedString("a", 1));
    ASSERT_EQ(received->getPayload().size(), (size_t)2);
    ASSERT_EQ(received->getPayload()[0], 'h');
    ASSERT_EQ(received->getPayload()[1], 'i');
    releasePacket(packet);
}

// Bug: Disconnect and Authentication stayed in the reason code state, so the
// properties length byte overwrote the reason code.
TEST(RegressionTests, DisconnectReasonCodeIsKept)
{
    MockClient client;
    const unsigned char disconnect[] = {0xE0, 0x02, 0x8E, 0x00}; // Session taken over
    client.pushToReadBuffer((void *)disconnect, sizeof(disconnect));

    Packet *packet = readPacketFromClient((Client *)&client);
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(((Disconnect *)packet)->getReasonCode(), 0x8E);
    releasePacket(packet);
}

// Bug: Disconnect reported itself complete as "needs more data", and a
// Disconnect with no properties never reached its complete state.
TEST(RegressionTests, DisconnectWithoutPropertiesIsReturned)
{
    MockClient client;
    const unsigned char disconnect[] = {0xE0, 0x01, 0x8E, 0xD0, 0x00}; // followed by a PINGRESP
    client.pushToReadBuffer((void *)disconnect, sizeof(disconnect));

    Packet *packet = readPacketFromClient((Client *)&client);
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(packet->getPacketType(), PacketId::DISCONNECT);
    ASSERT_EQ(((Disconnect *)packet)->getReasonCode(), 0x8E);
    releasePacket(packet);

    packet = readPacketFromClient((Client *)&client);
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(packet->getPacketType(), PacketId::PING_RESPONSE);
    releasePacket(packet);
}

TEST(RegressionTests, AuthenticationReasonCodeIsKept)
{
    MockClient client;
    const unsigned char auth[] = {0xF0, 0x02, 0x18, 0x00}; // Continue authentication
    client.pushToReadBuffer((void *)auth, sizeof(auth));

    Packet *packet = readPacketFromClient((Client *)&client);
    ASSERT_NE(packet, nullptr);
    ASSERT_EQ(((Authentication *)packet)->getReasonCode(), 0x18);
    releasePacket(packet);
}

// Bug: readPacketFromClient and Properties::readFromClient kept their parse
// state in function-level statics, so a partial packet on one connection
// corrupted parsing on another.
TEST(RegressionTests, TwoConnectionsParseIndependently)
{
    MockClient first, second;
    PacketReader firstReader, secondReader;

    // First connection stops part way through a Reason String property
    const unsigned char firstStart[] = {0x40, 0x08, 0x01, 0x02, 0x10, 0x04, 0x1F, 0x00};
    const unsigned char firstEnd[] = {0x01, 'x'};
    first.pushToReadBuffer((void *)firstStart, sizeof(firstStart));
    ASSERT_EQ(firstReader.read((Client *)&first), nullptr);

    // Second connection receives a whole packet with properties meanwhile
    const unsigned char secondPacket[] = {0x40, 0x08, 0x00, 0x09, 0x00, 0x04, 0x1F, 0x00, 0x01, 'y'};
    second.pushToReadBuffer((void *)secondPacket, sizeof(secondPacket));
    Packet *packet = secondReader.read((Client *)&second);
    ASSERT_NE(packet, nullptr);
    EXPECT_EQ(((PublishAcknowledge *)packet)->getPacketIdentifier(), 0x0009);
    releasePacket(packet);

    // The first connection then completes normally
    first.pushToReadBuffer((void *)firstEnd, sizeof(firstEnd));
    packet = firstReader.read((Client *)&first);
    ASSERT_NE(packet, nullptr);
    EXPECT_EQ(((PublishAcknowledge *)packet)->getPacketIdentifier(), 0x0102);
    EXPECT_EQ(((PublishAcknowledge *)packet)->getReasonCode(), 0x10);
    releasePacket(packet);
}

// Bug: publish() returns uint16_t, so its -1 failure value became 65535, a
// valid token. Packet Identifier 0 is never valid in MQTT, so 0 is the failure value.
TEST(RegressionTests, FailedPublishReturnsInvalidToken)
{
    MockClient client;
    MqttClient mqttClient((Client *)&client);
    EncodedString topic("t", 1);
    Payload payload;

    // Not connected, so the publish must fail
    uint16_t token = mqttClient.publish(topic, payload, QoS::ONE);
    EXPECT_EQ(token, PUBLISH_FAILED);
}

// Bug: subscribe() and unsubscribe() used C varargs, so only the first topic was sent.
TEST(RegressionTests, SubscribeSendsEveryTopic)
{
    MockClient client;
    MqttClient mqttClient((Client *)&client);
    setupConnected(client, mqttClient);
    client.clearWriteBuffer();

    SubscribePayload first, second;
    first.setTopic("a", 1);
    second.setTopic("b", 1);
    int token = mqttClient.subscribe(first, second);
    ASSERT_GT(token, 0);

    uint8_t *data = (uint8_t *)client.getWriteBuffer();
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(client.written(), (size_t)13);
    EXPECT_EQ(data[0], 0x82);                      // SUBSCRIBE with required flags
    EXPECT_EQ(data[1], 0x0B);                      // identifier 2 + properties 1 + 2 x (topic 3 + options 1)
    EXPECT_EQ((data[2] << 8) | data[3], token);    // Packet Identifier
    EXPECT_EQ(data[7], 'a');
    EXPECT_EQ(data[11], 'b');
}

TEST(RegressionTests, UnsubscribeSendsEveryTopic)
{
    MockClient client;
    MqttClient mqttClient((Client *)&client);
    setupConnected(client, mqttClient);
    client.clearWriteBuffer();

    UnsubscribePayload first, second;
    first.setTopic("a", 1);
    second.setTopic("b", 1);
    int token = mqttClient.unsubscribe(first, second);
    ASSERT_GT(token, 0);

    uint8_t *data = (uint8_t *)client.getWriteBuffer();
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(client.written(), (size_t)11);
    EXPECT_EQ(data[0], 0xA2); // UNSUBSCRIBE with required flags
    EXPECT_EQ(data[1], 0x09); // identifier 2 + properties 1 + 2 x topic 3
    EXPECT_EQ(data[7], 'a');
    EXPECT_EQ(data[10], 'b');
}

// Bug: a string was only read once all of it was available, so one larger
// than the socket's receive buffer could never be read.
TEST(RegressionTests, StringSplitAcrossReads)
{
    MockClient client;
    const unsigned char start[] = {0x00, 0x0A, 'h', 'e', 'l', 'l'};
    const unsigned char end[] = {'o', 'w', 'o', 'r', 'l', 'd'};
    EncodedString value;
    uint32_t read = 0;

    client.pushToReadBuffer((void *)start, sizeof(start));
    ASSERT_TRUE(value.readFromClient((Client *)&client, read));
    ASSERT_EQ(client.available(), 0) << "partial data should be consumed, not left waiting";

    client.pushToReadBuffer((void *)end, sizeof(end));
    ASSERT_FALSE(value.readFromClient((Client *)&client, read));
    ASSERT_EQ(read, (uint32_t)12);
    ASSERT_TRUE(value == EncodedString("helloworld", 10));
}

#ifdef STATIC_MEMORY
// Bug: with STATIC_MEMORY a string longer than the fixed buffer threw, which
// also fails to compile where exceptions are disabled. It is now truncated.
TEST(RegressionTests, StaticStringLongerThanBufferIsTruncated)
{
    MockClient client;
    const uint16_t size = MAX_BUFFER_SIZE + 88;
    unsigned char header[] = {(unsigned char)(size >> 8), (unsigned char)(size & 0xFF)};
    vector<unsigned char> body(size, 'x');
    client.pushToReadBuffer(header, sizeof(header));
    client.pushToReadBuffer(body.data(), body.size());

    EncodedString value;
    uint32_t read = 0;
    ASSERT_FALSE(value.readFromClient((Client *)&client, read));
    EXPECT_TRUE(value.wasTruncated());
    EXPECT_EQ((uint16_t)value.length, MAX_BUFFER_SIZE);
    EXPECT_EQ(read, (uint32_t)(size + 2));
    EXPECT_EQ(client.available(), 0) << "the discarded tail must still be consumed";
}
#endif

// Bug: sending DISCONNECT left the network connection open; MQTT 5 requires
// the sender to close it. This also applies to a keep alive timeout.
TEST(RegressionTests, DisconnectClosesConnection)
{
    MockClient client;
    MqttClient mqttClient((Client *)&client);
    setupConnected(client, mqttClient);

    ASSERT_EQ(mqttClient.disconnect(ReasonCode::NORMAL_DISCONNECTION), 0);
    EXPECT_EQ(client.stopped(), 1);
    EXPECT_FALSE(client.connected());
    EXPECT_FALSE(mqttClient.connected());
}

TEST(RegressionTests, KeepAliveTimeoutClosesConnection)
{
    MockClient client;
    MockMqttClient mqttClient((Client *)&client);
    mqttClient.setKeepAliveInterval(1);
    setupConnected(client, mqttClient);

    // 1.5 x the 1 second keep alive with no packet from the server
    mqttClient.setElapsedTime(1600);
    mqttClient.sync();

    uint8_t *data = (uint8_t *)client.getWriteBuffer();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(client.stopped(), 1);
    EXPECT_FALSE(client.connected());
}

// Bug: assigning to a Payload made with wrap() freed the caller's memory.
TEST(RegressionTests, AssigningToWrappedPayloadKeepsCallerMemory)
{
    uint8_t callerData[3] = {1, 2, 3};
    uint8_t other[2] = {9, 8};

    Payload wrapped = Payload::wrap(callerData, sizeof(callerData));
    ASSERT_EQ(wrapped.getData(), callerData) << "precondition: payload does not own the data";

    wrapped = Payload(other, sizeof(other)); // previously free()d callerData
    EXPECT_NE(wrapped.getData(), callerData);
    EXPECT_EQ(wrapped.size(), (size_t)2);
    EXPECT_EQ(wrapped[0], 9);
    EXPECT_EQ(callerData[0], 1);
}

// Bug: Payload::setData was declared but never defined. It copies with dynamic
// memory and refers to the caller's bytes with STATIC_MEMORY.
TEST(RegressionTests, PayloadSetData)
{
    uint8_t source[3] = {4, 5, 6};
    Payload payload;
    payload.setData(source, sizeof(source));
    source[0] = 0;

    EXPECT_EQ(payload.size(), (size_t)3);
    EXPECT_EQ(payload[2], 6);
#ifdef STATIC_MEMORY
    EXPECT_EQ(payload.getData(), source);
    EXPECT_EQ(payload[0], 0);
#else
    EXPECT_NE(payload.getData(), source);
    EXPECT_EQ(payload[0], 4);
#endif
}

// Bug: PacketBuffer::push had no bounds check, and with STATIC_MEMORY an
// oversized packet threw. It now refuses the write and reports overflow.
TEST(RegressionTests, PacketBufferRefusesOverflow)
{
    const uint8_t data[4] = {1, 2, 3, 4};
    uint8_t storage[2];
    PacketBuffer buffer(storage, sizeof(storage));
    EXPECT_EQ(buffer.push(data, 2), (size_t)2);
    EXPECT_FALSE(buffer.overflowed());
    EXPECT_EQ(buffer.push(data, 1), (size_t)0);
    EXPECT_TRUE(buffer.overflowed());
    EXPECT_EQ(buffer.getLength(), (size_t)2);
}

// Bug: packet readers reported the bytes they consumed as a negative
// (wrapped around) count.
TEST(RegressionTests, AcknowledgeReportsBytesConsumed)
{
    MockClient client;
    const unsigned char contents[] = {0x00, 0x05, 0x00}; // identifier and reason code
    client.pushToReadBuffer((void *)contents, sizeof(contents));

    PublishAcknowledge acknowledge;
    acknowledge.setRemainingLength((uint32_t)sizeof(contents));
    uint32_t read = 0;
    ASSERT_FALSE(acknowledge.readFromClient((Client *)&client, read));
    EXPECT_EQ(read, (uint32_t)sizeof(contents));
}

// Bug: readNextPacket reset the server keep alive timer before the CONNACK set
// the Server Keep Alive, so with no client keep alive the timer was 0 and the
// next sync with any time elapsed disconnected with a keep alive timeout.
TEST(RegressionTests, ServerKeepAliveDoesNotDisconnectImmediately)
{
    MockClient client;
    MockMqttClient mqttClient((Client *)&client);
    mqttClient.connect("localhost", 1883, 0);
    client.setIsConnected(true);
    mqttClient.connect("localhost", 1883, 0);
    mqttClient.sync(); // CONNECT, with the default keep alive of 0

    const unsigned char connack[] = {
        0x20, 0x06, 0x00, 0x00, 0x03, // CONNACK, properties length 3
        0x13, 0x00, 0x3C               // Server Keep Alive 60 seconds
    };
    client.pushToReadBuffer((void *)connack, sizeof(connack));
    mqttClient.sync();
    ASSERT_TRUE(mqttClient.connected());
    EXPECT_EQ(mqttClient.getKeepAliveInterval(), 60);

    client.clearWriteBuffer();
    mqttClient.setElapsedTime(10);
    mqttClient.sync();
    EXPECT_TRUE(mqttClient.connected());
    EXPECT_EQ(client.stopped(), 0);
    EXPECT_EQ(client.getWriteBuffer(), nullptr) << "nothing should be sent 10 ms after connecting";
}

// Bug: on PICO builds MqttClient treated any non-zero connect() result as a
// failure and elsewhere only 0, so a client returning 1 (Arduino success) or a
// negative error was misread on one platform or the other. Now <= 0 is failure.
namespace
{
    class ConnectResultClient : public FixedBufferClient
    {
    public:
        int result = 1;
        int connect(const char *host, uint16_t port) override
        {
            FixedBufferClient::connect(host, port);
            return result;
        }
    };
}

TEST(RegressionTests, ConnectResultConvention)
{
    ConnectResultClient success;
    MqttClient succeeding(&success);
    EXPECT_EQ(succeeding.connect("broker.local", 1883, 1000), 0);

    ConnectResultClient failure;
    failure.result = -1;
    MqttClient failing(&failure);
    EXPECT_EQ(failing.connect("broker.local", 1883, 1000), -1);

    ConnectResultClient zero;
    zero.result = 0;
    MqttClient alsoFailing(&zero);
    EXPECT_EQ(alsoFailing.connect("broker.local", 1883, 1000), -1);
}
