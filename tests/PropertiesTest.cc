#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "utils/TestPacketBuffer.h"
#include "stdint.h"
#include <string>

#include "mocks/MockClient.h"
#include "MqttProperties.h"

using namespace std;

using namespace CppMqtt;

#define PAYLOAD_FORMAT_INDICATOR_PROPERTY 1, 0
#define MESSAGE_EXPIRY_INTERVAL_PROPERTY 2, 0, 0, 0, 0
#define CONTENT_TYPE_PROPERTY 3, 0, 0
#define RESPONSE_TOPIC_PROPERTY 8, 0, 0
#define CORRELATION_DATA_PROPERTY 9, 0, 0
#define SUBSCRIPTION_IDENTIFIER_PROPERTY 11, 0
#define SESSION_EXPIRY_INTERVAL_PROPERTY 17, 0, 0, 0, 0
#define ASSIGNED_CLIENT_IDENTIFER_PROPERTY 18, 0, 0
#define SERVER_KEEP_ALIVE_PROPERTY 19, 0, 0
#define AUTHENTICATION_METHOD_PROPERTY 21, 0, 0
#define AUTHENTICATION_DATA_PROPERTY 22, 0, 0
#define REQUEST_PROBLEM_INFORMATION_PROPERTY 23, 0
#define WILL_DELAY_INTERVAL_PROPERTY 24, 0, 0, 0, 0
#define REQUEST_RESPONSE_INFORMATION_PROPERTY 25, 0
#define RESPONSE_INFORMATION_PROPERTY 26, 0, 0
#define SERVER_REFERENCE_PROPERTY 28, 0, 0
#define REASON_STRING_PROPERTY 31, 0, 0
#define RECEIVE_MAXIMUM_PROPERTY 33, 0, 0
#define TOPIC_ALIAS_MAXIMUM_PROPERTY 34, 0, 0
#define TOPIC_ALIAS_PROPERTY 35, 0, 0
#define MAXIMUM_QOS_PROPERTY 36, 0
#define RETAIN_AVAILABLE_PROPERTY 37, 0
#define USER_PROPERTY_PROPERTY 38, 0, 0, 0, 0
#define MAXIMUM_PACKET_SIZE_PROPERTY 39, 0, 0, 0, 0
#define WILDCARD_SUBSCRIPTION_AVAILABLE_PROPERTY 40, 0
#define SUBSCRIPTION_IDENTIFIERS_AVAILABLE_PROPERTY 41, 0
#define SHARED_SUBSCRIPTION_AVAILABLE_PROPERTY 241, 1, 0

#define TEST_DATA_SIZE 84

static uint8_t testData[] = {
    83,
    PAYLOAD_FORMAT_INDICATOR_PROPERTY,
    MESSAGE_EXPIRY_INTERVAL_PROPERTY,
    CONTENT_TYPE_PROPERTY,
    RESPONSE_TOPIC_PROPERTY,
    CORRELATION_DATA_PROPERTY,
    SUBSCRIPTION_IDENTIFIER_PROPERTY,
    SESSION_EXPIRY_INTERVAL_PROPERTY,
    ASSIGNED_CLIENT_IDENTIFER_PROPERTY,
    SERVER_KEEP_ALIVE_PROPERTY,
    AUTHENTICATION_METHOD_PROPERTY,
    AUTHENTICATION_DATA_PROPERTY,
    REQUEST_PROBLEM_INFORMATION_PROPERTY,
    WILL_DELAY_INTERVAL_PROPERTY,
    REQUEST_RESPONSE_INFORMATION_PROPERTY,
    RESPONSE_INFORMATION_PROPERTY,
    SERVER_REFERENCE_PROPERTY,
    REASON_STRING_PROPERTY,
    RECEIVE_MAXIMUM_PROPERTY,
    TOPIC_ALIAS_MAXIMUM_PROPERTY,
    TOPIC_ALIAS_PROPERTY,
    MAXIMUM_QOS_PROPERTY,
    RETAIN_AVAILABLE_PROPERTY,
    USER_PROPERTY_PROPERTY,
    MAXIMUM_PACKET_SIZE_PROPERTY,
    WILDCARD_SUBSCRIPTION_AVAILABLE_PROPERTY,
    SUBSCRIPTION_IDENTIFIERS_AVAILABLE_PROPERTY,
    SHARED_SUBSCRIPTION_AVAILABLE_PROPERTY};

TEST(PropertiesTest, Decode)
{
#ifdef STATIC_MEMORY
    // One of every property type: 27 properties in one packet
    if (CPP_MQTT_MAX_PROPERTIES < 27 || CPP_MQTT_PROPERTY_POOL_SIZE < 27)
    {
        GTEST_SKIP() << "Needs CPP_MQTT_MAX_PROPERTIES and CPP_MQTT_PROPERTY_POOL_SIZE of at least 27";
    }
#endif

    for (size_t i = 0; i < 1; i++)
    {
        MockClient client;
        Properties properties;

        Client *clientPtr = (Client *)&client;

        uint32_t bytesRead = 0;

        client.pushToReadBuffer(testData, TEST_DATA_SIZE);

        ASSERT_EQ(properties.length(), 0);

        properties.readFromClient(clientPtr, bytesRead);

        ASSERT_EQ(properties.length(), 27);
    }
}

TEST(PropertiesTest, Encode)
{
#ifdef STATIC_MEMORY
    // One of every property type: 27 properties in one packet
    if (CPP_MQTT_MAX_PROPERTIES < 27 || CPP_MQTT_PROPERTY_POOL_SIZE < 27)
    {
        GTEST_SKIP() << "Needs CPP_MQTT_MAX_PROPERTIES and CPP_MQTT_PROPERTY_POOL_SIZE of at least 27";
    }
#endif

    for (size_t i = 0; i < 1; i++)
    {
        MockClient client;
        Properties properties;

        ASSERT_NE(properties.add<PayloadFormatIndicatorProperty>(), nullptr);
        ASSERT_NE(properties.add<MessageExpiryIntervalProperty>(), nullptr);
        ASSERT_NE(properties.add<ContentTypeProperty>(), nullptr);
        ASSERT_NE(properties.add<ResponseTopic>(), nullptr);
        ASSERT_NE(properties.add<CorrelationData>(), nullptr);
        ASSERT_NE(properties.add<SubscriptionIdentifier>(), nullptr);
        ASSERT_NE(properties.add<SessionExpiryInterval>(), nullptr);
        ASSERT_NE(properties.add<AssignedClientIdentifier>(), nullptr);
        ASSERT_NE(properties.add<ServerKeepAlive>(), nullptr);
        ASSERT_NE(properties.add<AuthenticationMethod>(), nullptr);
        ASSERT_NE(properties.add<AuthenticationData>(), nullptr);
        ASSERT_NE(properties.add<RequestProblemInformation>(), nullptr);
        ASSERT_NE(properties.add<WillDelayInterval>(), nullptr);
        ASSERT_NE(properties.add<RequestResponseInformation>(), nullptr);
        ASSERT_NE(properties.add<ResponseInformation>(), nullptr);
        ASSERT_NE(properties.add<ServerReference>(), nullptr);
        ASSERT_NE(properties.add<ReasonString>(), nullptr);
        ASSERT_NE(properties.add<ReceiveMaxium>(), nullptr);
        ASSERT_NE(properties.add<TopicAliasMaximum>(), nullptr);
        ASSERT_NE(properties.add<TopicAlias>(), nullptr);
        ASSERT_NE(properties.add<MaxiumumQoS>(), nullptr);
        ASSERT_NE(properties.add<RetainAvailable>(), nullptr);
        ASSERT_NE(properties.add<UserProperty>(), nullptr);
        ASSERT_NE(properties.add<MaximumPacketSize>(), nullptr);
        ASSERT_NE(properties.add<WildcardSubscriptionAvailable>(), nullptr);
        ASSERT_NE(properties.add<SubscriptionIdentifierAvailable>(), nullptr);
        ASSERT_NE(properties.add<SharedSubscriptionAvailable>(), nullptr);

        Client *clientPtr = (Client *)&client;

        uint32_t bytesRead = 0;

        TestPacketBuffer buffer;
        size_t written = properties.push(buffer);

        ASSERT_EQ(written, TEST_DATA_SIZE);

        uint8_t *data = (uint8_t *)buffer.getBuffer();

        for (size_t i = 0; i < TEST_DATA_SIZE; i++)
        {
            ASSERT_EQ(data[i], testData[i]) << "at position " << i;
        }
    }
}