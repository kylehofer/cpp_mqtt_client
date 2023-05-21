#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"

#include "mocks/TestTcpClient.h"
#include "MqttClient.h"

#include <thread>
#include <chrono>

const char CLIENT_ADDRESS[] = "tcp://localhost:1883";
const char CLIENT_CLIENT_ID[] = "unique_id";
const int BROKER_PORT = 1883;
const char BROKER_HOSTNAME[] = "localhost";

#define WAIT_TIMEOUT 15

using namespace std;
using namespace PicoMqtt;
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
            brokerThread = new thread(&system, "mosquitto");
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

TEST_F(TestTcpClientTests, EndToEndTrial)
{

    TestTcpClient client(BROKER_PORT);
    Client *clientPtr = (Client *)&client;
    size_t bytesWritten;

    MqttClient mqttClient(clientPtr);

    mqttClient.setKeepAliveInterval(1500);

    mqttClient.connect("localhost", BROKER_PORT, 0);

    char buffer[50];

    for (size_t i = 0; i < 10000; i++)
    {
        client.sync();
        client.flush();
        mqttClient.sync();
        if (i > 100)
        {
            int size = sprintf(buffer, "Publish %ld", i);
            EncodedString topic("my/topic", 8);
            Payload payload(buffer, size);
            uint16_t token = mqttClient.publish(topic, payload, QoS::TWO);
        }
        sleep_for(milliseconds(WAIT_TIMEOUT));
    }
}