#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"
#include <string>

#include "mocks/MockClient.h"
#include "PicoMqttProperties.h"

using namespace std;

using namespace PicoMqtt;

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

    for (size_t i = 0; i < 1; i++)
    {
        MockClient client;
        Properties properties;

        properties.addProperty((Property *)new PayloadFormatIndicatorProperty());
        properties.addProperty((Property *)new MessageExpiryIntervalProperty());
        properties.addProperty((Property *)new ContentTypeProperty());
        properties.addProperty((Property *)new ResponseTopic());
        properties.addProperty((Property *)new CorrelationData());
        properties.addProperty((Property *)new SubscriptionIdentifier());
        properties.addProperty((Property *)new SessionExpiryInterval());
        properties.addProperty((Property *)new AssignedClientIdentifier());
        properties.addProperty((Property *)new ServerKeepAlive());
        properties.addProperty((Property *)new AuthenticationMethod());
        properties.addProperty((Property *)new AuthenticationData());
        properties.addProperty((Property *)new RequestProblemInformation());
        properties.addProperty((Property *)new WillDelayInterval());
        properties.addProperty((Property *)new RequestResponseInformation());
        properties.addProperty((Property *)new ResponseInformation());
        properties.addProperty((Property *)new ServerReference());
        properties.addProperty((Property *)new ReasonString());
        properties.addProperty((Property *)new ReceiveMaxium());
        properties.addProperty((Property *)new TopicAliasMaximum());
        properties.addProperty((Property *)new TopicAlias());
        properties.addProperty((Property *)new MaxiumumQoS());
        properties.addProperty((Property *)new RetainAvailable());
        properties.addProperty((Property *)new UserProperty());
        properties.addProperty((Property *)new MaximumPacketSize());
        properties.addProperty((Property *)new WildcardSubscriptionAvailable());
        properties.addProperty((Property *)new SubscriptionIdentifierAvailable());
        properties.addProperty((Property *)new SharedSubscriptionAvailable());

        Client *clientPtr = (Client *)&client;

        uint32_t bytesRead = 0;

        PacketBuffer buffer(properties.size());
        size_t written = properties.push(buffer);

        ASSERT_EQ(written, TEST_DATA_SIZE);

        uint8_t *data = (uint8_t *)buffer.getBuffer();

        for (size_t i = 0; i < TEST_DATA_SIZE; i++)
        {
            ASSERT_EQ(data[i], testData[i]) << "at position " << i;
        }
    }
}