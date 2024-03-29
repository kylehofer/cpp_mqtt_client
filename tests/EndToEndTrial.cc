#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"

#include "mocks/TestTcpClient.h"
#include "MqttClient.h"
#include "types/SubscribePayload.h"
#include "utils/MosquittoTestClient.h"
#include "utils/MqttTestHandler.h"

#include <functional>
#include <thread>
#include <chrono>

const char CLIENT_ADDRESS[] = "tcp://localhost:1883";
const char CLIENT_CLIENT_ID[] = "unique_id";
const int BROKER_PORT = 1883;
const char BROKER_HOSTNAME[] = "localhost";

#define WAIT_TIMEOUT 15

#define MAX_RETRIES 400
#define RETRY_TIMEOUT 5

using namespace std;
using namespace CppMqtt;
using ::chrono::microseconds;
using ::chrono::milliseconds;
using ::this_thread::sleep_for;

static thread *brokerThread = nullptr;

class TestTcpClientTests : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {

        if (brokerThread == nullptr)
        {
#ifdef DEBUG_TESTS
            brokerThread = new thread(&system, "mosquitto -v");
#else
            brokerThread = new thread(&system, "mosquitto");
#endif

            TestTcpClient::portCheck(BROKER_PORT);
        }
    }

    static void TearDownTestSuite()
    {
        system("killall mosquitto");
        brokerThread->join();
        delete brokerThread;
        brokerThread = nullptr;
    }

    void SetUp() override
    {
    }

    // You can define per-test tear-down logic as usual.
    void TearDown() override
    {
    }
};

/**
 * @brief Utility method for checking if data has been recieved by a test client.
 *
 * @param testClient
 * @param sync
 * @param topic
 * @param data
 */
inline void expectData(MosquittoTestClient *testClient, function<void()> sync, const char *topic, const uint8_t *data)
{
    int retries = 0;

    while (!testClient->hasData() && retries++ < MAX_RETRIES)
    {
        sync();
        this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    ASSERT_TRUE(testClient->hasData());
    PayloadMessage receivedMessage;
    receivedMessage = testClient->pop();
    EXPECT_STREQ(receivedMessage.topic, topic);
    for (size_t i = 0; i < receivedMessage.payloadLength; i++)
    {
        EXPECT_EQ(receivedMessage.payload[i], data[i]) << "at position " << i;
    }
    MosquittoTestClient::freePayload(receivedMessage);

    sync();
}

// Wraps the expectData method with a SCOPED_TRACE
#define EXPECT_DATA(testClient, sync, topic, data) \
    {                                              \
        SCOPED_TRACE("DATA_CHECK");                \
        expectData(testClient, sync, topic, data); \
    }

TEST_F(TestTcpClientTests, E2EPublish)
{
    TestTcpClient client(BROKER_PORT);
    Client *clientPtr = (Client *)&client;
    size_t bytesWritten;

    MosquittoTestClient testClient;

    MqttClient mqttClient(clientPtr);

    mqttClient.setKeepAliveInterval(15500);

    mqttClient.connect("localhost", BROKER_PORT, 0);
    testClient.connect("localhost", BROKER_PORT);

    auto sync = [&client, &mqttClient]()
    {
        client.sync();
        mqttClient.sync();
    };

    while (!mqttClient.connected())
    {
        client.sync();
        mqttClient.sync();
    }

    testClient.waitForReady(MAX_RETRIES);

    QoS qosValues[] = {QoS::ZERO, QoS::ONE, QoS::TWO};

    for (auto qos : qosValues)
    {
        char stringBuffer[] = "word";
        EncodedString topic("my/topic", 8);
        Payload payload(stringBuffer, 5);
        uint16_t token = mqttClient.publish(topic, payload, qos);

        uint8_t data[] = {'w', 'o', 'r', 'd', '\0'};

        EXPECT_DATA(&testClient, sync, "my/topic", data);

        topic = EncodedString("another/topic", 13);

        uint8_t rawBuffer[] = {100, 101, 102};
        payload = Payload(rawBuffer, 3);
        token = mqttClient.publish(topic, payload, qos);

        uint8_t rawData[] = {100, 101, 102};

        EXPECT_DATA(&testClient, sync, "another/topic", rawData);
    }
}

TEST_F(TestTcpClientTests, E2ESubscribe)
{
    TestTcpClient client(BROKER_PORT);
    Client *clientPtr = (Client *)&client;
    size_t bytesWritten;

    MosquittoTestClient testClient;

    MqttClient mqttClient(clientPtr);

    MqttTestHandler listener;

    mqttClient.setHandler((MqttClientHandler *)&listener);

    mqttClient.setKeepAliveInterval(15500);

    mqttClient.connect("localhost", BROKER_PORT, 0);
    testClient.connect("localhost", BROKER_PORT);

    auto sync = [&client, &mqttClient]()
    {
        mqttClient.sync();
        client.sync();
    };

    int retries = 0;
    while (!mqttClient.connected() && retries++ < 500)
    {
        sync();
    }

    ASSERT_TRUE(mqttClient.connected());

    testClient.waitForReady(MAX_RETRIES);

    int qosValues[] = {0, 1, 2};

    SubscribePayload subPayload;
    subPayload.setTopic("#", 1);
    Token subscribeToken = mqttClient.subscribe(subPayload);

    retries = 0;
    while (!listener.subscribeResult.contains(subscribeToken) && retries++ < 500)
    {
        sync();
    }

    ASSERT_TRUE(listener.subscribeResult.contains(subscribeToken));

    for (auto qos : qosValues)
    {
        testClient.publish("my/topic", (void *)"word", 5, qos);

        retries = 0;
        while (listener.topicQueue.size() == 0 && retries++ < 500)
        {
            testClient.isReady();
            sync();
        }

        ASSERT_TRUE(listener.topicQueue.size() > 0);

        EncodedString topic = listener.topicQueue.front();
        Payload payload = listener.payloadQueue.front();

        listener.topicQueue.pop();
        listener.payloadQueue.pop();

        char expectedTopic[] = "my/topic";
        uint8_t expectedData[] = {'w', 'o', 'r', 'd', '\0'};

        for (size_t i = 0; i < topic.length; i++)
        {
            EXPECT_EQ(topic[i], expectedTopic[i]) << "at position " << i;
        }

        for (size_t i = 0; i < payload.size(); i++)
        {
            EXPECT_EQ(payload[i], expectedData[i]) << "at position " << i;
        }
    }
}

TEST_F(TestTcpClientTests, E2EUnsubscribe)
{
    TestTcpClient client(BROKER_PORT);
    Client *clientPtr = (Client *)&client;
    size_t bytesWritten;

    MosquittoTestClient testClient;

    MqttClient mqttClient(clientPtr);

    MqttTestHandler listener;

    mqttClient.setHandler((MqttClientHandler *)&listener);

    mqttClient.setKeepAliveInterval(15500);

    mqttClient.connect("localhost", BROKER_PORT, 0);
    testClient.connect("localhost", BROKER_PORT);

    auto sync = [&client, &mqttClient]()
    {
        mqttClient.sync();
        client.sync();
    };

    int retries = 0;
    while (!mqttClient.connected() && retries++ < 500)
    {
        sync();
    }

    ASSERT_TRUE(mqttClient.connected());

    testClient.waitForReady(MAX_RETRIES);

    int qosValues[] = {0, 1, 2};

    SubscribePayload subPayload;
    subPayload.setTopic("#", 1);
    Token subscribeToken = mqttClient.subscribe(subPayload);

    retries = 0;
    while (!listener.subscribeResult.contains(subscribeToken) && retries++ < 500)
    {
        sync();
    }

    ASSERT_TRUE(listener.subscribeResult.contains(subscribeToken));

    UnsubscribePayload unsubPayload;
    unsubPayload.setTopic("#", 1);
    Token unsubscribeToken = mqttClient.unsubscribe(unsubPayload);

    retries = 0;
    while (!listener.unsubscribeResult.contains(unsubscribeToken) && retries++ < 500)
    {
        sync();
    }

    ASSERT_TRUE(listener.unsubscribeResult.contains(unsubscribeToken));

    for (auto qos : qosValues)
    {
        testClient.publish("my/topic", (void *)"word", 5, qos);

        retries = 0;
        while (listener.topicQueue.size() == 0 && retries++ < 10)
        {
            testClient.isReady();
            sync();
        }

        ASSERT_FALSE(listener.topicQueue.size() > 0);
    }
}
