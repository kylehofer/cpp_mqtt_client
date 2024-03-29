#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"

#include "../mocks/MockClient.h"
#include "MqttProperties.h"

using namespace std;

using namespace CppMqtt;

struct ByteTestData
{
    uint8_t raw;
    uint8_t encoded;
};

static vector<ByteTestData> testData({
    {0, 0x0},
    {127, 0x7F},
    {255, 0xFF},
});

template <typename T>
class BytePropertyTest : public ::testing::Test
{
};

TYPED_TEST_SUITE_P(BytePropertyTest);

TYPED_TEST_P(BytePropertyTest, Encode)
{
    ByteProperty *byteProperty = (ByteProperty *)new TypeParam();
    VariableByteInteger identifier = byteProperty->getIdentifier();

    for (auto &data : testData)
    {
        MockClient client;
        Client *clientPtr = (Client *)&client;
        size_t bytesWritten;

        PacketBuffer buffer(byteProperty->size());

        byteProperty->setValue(data.raw);
        bytesWritten = byteProperty->push(buffer);

        char *writeBuffer = client.getWriteBuffer();

        ASSERT_EQ(bytesWritten, 1 + identifier.size());

        // uint8_t *identifierBuffer = buffer.getBuffer();
        uint8_t *byteBuffer = buffer.getBuffer() + identifier.size();

        // for (size_t i = 0; i < identifier.size(); i++)
        // {
        //     ASSERT_EQ((uint8_t)identifierBuffer[i], (uint8_t)testData.identifier[i]) << "Buffers are incorrect at index " << i;
        // }

        ASSERT_EQ((uint8_t)byteBuffer[0], data.encoded);
    }

    delete byteProperty;
}

TYPED_TEST_P(BytePropertyTest, Decode)
{
    ByteProperty *byteProperty = (ByteProperty *)new TypeParam();
    VariableByteInteger identifier = byteProperty->getIdentifier();

    for (auto &data : testData)
    {
        MockClient client;
        Client *clientPtr = (Client *)&client;
        uint32_t bytesRead = 0;
        char *readBuffer;

        client.pushToReadBuffer(&data.encoded, 1);

        byteProperty->readFromClient(clientPtr, bytesRead);

        ASSERT_EQ(bytesRead, 1);

        ASSERT_EQ(data.raw, byteProperty->getValue());
    }
    delete byteProperty;
}

REGISTER_TYPED_TEST_SUITE_P(BytePropertyTest, Encode, Decode);

using PropertyTypes = ::testing::Types<PayloadFormatIndicatorProperty, RequestProblemInformation, RequestResponseInformation, MaxiumumQoS, RetainAvailable, WildcardSubscriptionAvailable, WildcardSubscriptionAvailable, SharedSubscriptionAvailable>;

INSTANTIATE_TYPED_TEST_SUITE_P(My, BytePropertyTest, PropertyTypes);
