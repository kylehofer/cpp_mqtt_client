/*
 * File: MqttClientTests.cc
 * Project: cpp_mqtt_client
 * Created Date: Sunday April 23rd 2023
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

#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"

#include "mocks/MockClient.h"
#include "PicoMqttProperties.h"
#include "MqttClient.h"
#include "mocks/MockMqttClient.h"
#include "packets/ConnectAcknowledge.h"

using namespace std;

using namespace PicoMqtt;

TEST(MqttClientTests, SuccessfulConnect)
{

    MockClient client;
    Client *clientPtr = (Client *)&client;
    size_t bytesWritten;

    MqttClient mqttClient(clientPtr);

    mqttClient.connect("localhost", 1883, 0);

    ASSERT_FALSE(mqttClient.connected());
    ASSERT_EQ(client.getWriteBuffer(), nullptr);

    client.setIsConnected(true);

    mqttClient.connect("localhost", 1883, 0);

    ASSERT_FALSE(mqttClient.connected());
    ASSERT_NE(client.getWriteBuffer(), nullptr);

    char *writeBuffer = client.getWriteBuffer();

    uint8_t expectedData[] = {
        0x10, // Connect ID
        0x0D, // Remaining Length
        0x00,
        0x04,
        'M',
        'Q',
        'T',
        'T',
        0x05,  // Protocol
        0x00,  // Flags
        0x00,  // Keep Alive Upper
        0x00,  // Keep Alive Lower
        0x00}; // Properties Length

    for (size_t i = 0; i < 11; i++)
    {
        ASSERT_EQ(writeBuffer[i], expectedData[i]) << "at position " << i;
    }

    mqttClient.sync();

    ASSERT_FALSE(mqttClient.connected());

    ConnectAcknowledge connectAcknowledge;

    const unsigned char connack[] = {
        0x20, 0x03, // Variable Length
        0x00,       // Flags
        0x00,       // Success
        0x00        // No properties
    };

    client.pushToReadBuffer((void *)connack, 5);

    mqttClient.sync();

    ASSERT_TRUE(mqttClient.connected());
}

TEST(MqttClientTests, FailedConnect)
{
    MockClient client;
    Client *clientPtr = (Client *)&client;
    size_t bytesWritten;

    MqttClient mqttClient(clientPtr);

    mqttClient.connect("localhost", 1883, 0);

    ASSERT_FALSE(mqttClient.connected());
    ASSERT_EQ(client.getWriteBuffer(), nullptr);

    client.setIsConnected(true);

    mqttClient.connect("localhost", 1883, 0);

    ASSERT_FALSE(mqttClient.connected());
    ASSERT_NE(client.getWriteBuffer(), nullptr);

    char *writeBuffer = client.getWriteBuffer();

    uint8_t expectedData[] = {
        0x10, // Connect ID
        0x0D, // Remaining Length
        0x00,
        0x04,
        'M',
        'Q',
        'T',
        'T',
        0x05,  // Protocol
        0x00,  // Flags
        0x00,  // Keep Alive Upper
        0x00,  // Keep Alive Lower
        0x00}; // Properties Length

    for (size_t i = 0; i < 11; i++)
    {
        ASSERT_EQ(writeBuffer[i], expectedData[i]) << "at position " << i;
    }

    ASSERT_EQ(client.written(), 15);

    mqttClient.sync();

    ASSERT_FALSE(mqttClient.connected());

    ConnectAcknowledge connectAcknowledge;

    const unsigned char connack[] = {
        0x20, 0x03, // Variable Length
        0x00,       // Flags
        0xf0,       // Success
        0x00        // No properties
    };

    client.pushToReadBuffer((void *)connack, 5);

    mqttClient.sync();

    ASSERT_FALSE(mqttClient.connected());
}

void setupConnected(MockClient &client, MqttClient &mqttClient)
{
    mqttClient.connect("localhost", 1883, 0);

    ASSERT_FALSE(mqttClient.connected());
    ASSERT_EQ(client.getWriteBuffer(), nullptr);

    client.setIsConnected(true);

    mqttClient.connect("localhost", 1883, 0);

    ASSERT_FALSE(mqttClient.connected());
    ASSERT_NE(client.getWriteBuffer(), nullptr);

    char *writeBuffer = client.getWriteBuffer();

    uint8_t expectedData[] = {
        0x10, // Connect ID
        0x0D, // Remaining Length
        0x00,
        0x04,
        'M',
        'Q',
        'T',
        'T',
        0x05,  // Protocol
        0x00,  // Flags
        0x00,  // Keep Alive Upper
        0x00,  // Keep Alive Lower
        0x00}; // Properties Length

    for (size_t i = 0; i < 11; i++)
    {
        ASSERT_EQ(writeBuffer[i], expectedData[i]) << "at position " << i;
    }

    client.clearWriteBuffer();

    mqttClient.sync();

    ASSERT_FALSE(mqttClient.connected());

    ConnectAcknowledge connectAcknowledge;

    const unsigned char connack[] = {
        0x20, 0x03, // Variable Length
        0x00,       // Flags
        0x00,       // Success
        0x00        // No properties
    };

    client.pushToReadBuffer((void *)connack, 5);

    mqttClient.sync();

    ASSERT_TRUE(mqttClient.connected());
}

TEST(MqttClientTests, PublishQos0)
{
    MockClient client;
    Client *clientPtr = (Client *)&client;

    MqttClient mqttClient(clientPtr);

    setupConnected(client, mqttClient);

    EncodedString topic("my/topic", 8);
    Payload payload;

    uint16_t token = mqttClient.publish(topic, payload, QoS::ZERO);

    ASSERT_TRUE(mqttClient.isDelivered(token));

    ASSERT_NE(client.getWriteBuffer(), nullptr);

    char *writeBuffer = client.getWriteBuffer();

    uint8_t expectedData[] = {
        0x30, // Connect ID
        0x0B, // Remaining Length
        0x00, // Length of topic (Big Endian Ordering)
        0x08,
        'm', 'y', '/', 't', 'o', 'p', 'i', 'c',
        0x00}; // Properties Length

    for (size_t i = 0; i < 13; i++)
    {
        ASSERT_EQ(writeBuffer[i], expectedData[i]) << "at position " << i;
    }

    client.clearWriteBuffer();
}

TEST(MqttClientTests, PublishQos1)
{
    MockClient client;
    Client *clientPtr = (Client *)&client;

    MqttClient mqttClient(clientPtr);

    setupConnected(client, mqttClient);

    EncodedString topic("my/topic", 8);
    Payload payload;

    uint16_t token = mqttClient.publish(topic, payload, QoS::ONE);

    ASSERT_FALSE(mqttClient.isDelivered(token));

    const unsigned char puback[] = {
        0x40,                    // ID
        0x04,                    // Variable Length
        (uint8_t)(token & 0xFF), // Packet Identifier Lower
        (uint8_t)(token >> 8),   // Packet Identifier Upper
        0x00,                    // Reason Code
        0x00                     // No properties
    };

    client.pushToReadBuffer((void *)puback, 6);

    mqttClient.sync();

    ASSERT_TRUE(mqttClient.isDelivered(token));
}

TEST(MqttClientTests, PublishQos2)
{
    MockClient client;
    Client *clientPtr = (Client *)&client;

    MqttClient mqttClient(clientPtr);

    setupConnected(client, mqttClient);

    EncodedString topic("my/topic", 8);
    Payload payload;

    uint16_t token = mqttClient.publish(topic, payload, QoS::TWO);

    ASSERT_FALSE(mqttClient.isDelivered(token));

    const unsigned char pubrec[] = {
        0x50,                    // ID
        0x04,                    // Variable Length
        (uint8_t)(token & 0xFF), // Packet Identifier Lower
        (uint8_t)(token >> 8),   // Packet Identifier Upper
        0x00,                    // Reason Code
        0x00                     // No properties
    };

    client.pushToReadBuffer((void *)pubrec, 6);

    mqttClient.sync();

    ASSERT_FALSE(mqttClient.isDelivered(token));

    const unsigned char pubcomp[] = {
        0x70,                    // ID
        0x04,                    // Variable Length
        (uint8_t)(token & 0xFF), // Packet Identifier Lower
        (uint8_t)(token >> 8),   // Packet Identifier Upper
        0x00,                    // Reason Code
        0x00                     // No properties
    };

    client.pushToReadBuffer((void *)pubcomp, 6);

    mqttClient.sync();

    ASSERT_TRUE(mqttClient.isDelivered(token));
}

TEST(MqttClientTests, TestKeepAlivePeriod)
{
    MockClient client;
    Client *clientPtr = (Client *)&client;

    MockMqttClient mqttClient(clientPtr);

    mqttClient.setKeepAliveInterval(500);

    setupConnected(client, mqttClient);

    client.clearWriteBuffer();

    mqttClient.setElapsedTime(500);

    mqttClient.sync();

    ASSERT_NE(client.getWriteBuffer(), nullptr);

    char *writeBuffer = client.getWriteBuffer();

    uint8_t expectedData[] = {
        0xC0, // Ping ID
        0x00, // Remaining Length
    };

    for (size_t i = 0; i < 2; i++)
    {
        ASSERT_EQ(writeBuffer[i], (char)expectedData[i]) << "at position " << i;
    }

    client.clearWriteBuffer();

    mqttClient.setElapsedTime(250);

    mqttClient.sync();

    ASSERT_NE(client.getWriteBuffer(), nullptr);

    writeBuffer = client.getWriteBuffer();

    uint8_t disconnectPacket[] = {
        0xE0, // Ping ID
        0x01, // Remaining Length
        0x8D, // Reason Code: 141 Timeout
    };

    for (size_t i = 0; i < 3; i++)
    {
        ASSERT_EQ(writeBuffer[i], (char)disconnectPacket[i]) << "at position " << i;
    }
}