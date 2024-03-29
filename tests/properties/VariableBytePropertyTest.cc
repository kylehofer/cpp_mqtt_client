#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"

#include "../mocks/MockClient.h"
#include "MqttProperties.h"

using namespace std;

using namespace CppMqtt;

struct VariableByteTestData
{
    uint32_t raw;
    const uint8_t *encoded;
    size_t encodedSize;
};

const uint8_t NONE_BUFFER[] = {0x0};
const uint8_t ONE_MIN_BUFFER[] = {0x1};
const uint8_t ONE_MAX_BUFFER[] = {0x7F};
const uint8_t TWO_MIN_BUFFER[] = {0x80, 0x1};
const uint8_t TWO_MAX_BUFFER[] = {0xFF, 0x7F};
const uint8_t THREE_MIN_BUFFER[] = {0x80, 0x80, 0x1};
const uint8_t THREE_MAX_BUFFER[] = {0xFF, 0xFF, 0x7F};
const uint8_t FOUR_MIN_BUFFER[] = {0x80, 0x80, 0x80, 0x1};
const uint8_t FOUR_MAX_BUFFER[] = {0xFF, 0xFF, 0xFF, 0x7F};

static vector<VariableByteTestData> testData{
    {0, NONE_BUFFER, 1},
    {1, ONE_MIN_BUFFER, 1},
    {127, ONE_MAX_BUFFER, 1},
    {128, TWO_MIN_BUFFER, 2},
    {16383, TWO_MAX_BUFFER, 2},
    {16384, THREE_MIN_BUFFER, 3},
    {2097151, THREE_MAX_BUFFER, 3},
    {2097152, FOUR_MIN_BUFFER, 4},
    {268435455, FOUR_MAX_BUFFER, 4},
};

template <typename T>
class VariableBytePropertyTest : public ::testing::Test
{
};

TYPED_TEST_SUITE_P(VariableBytePropertyTest);

TYPED_TEST_P(VariableBytePropertyTest, Encode)
{
    VariableByteProperty *variableByteProperty = (VariableByteProperty *)new TypeParam();
    VariableByteInteger identifier = variableByteProperty->getIdentifier();

    for (auto &data : testData)
    {
        MockClient client;
        Client *clientPtr = (Client *)&client;
        size_t bytesWritten;

        variableByteProperty->setValue(VariableByteInteger(data.raw));
        PacketBuffer packetBuffer(variableByteProperty->size());
        bytesWritten = variableByteProperty->push(packetBuffer);

        char *writeBuffer = (char *)packetBuffer.getBuffer();

        ASSERT_EQ(bytesWritten, data.encodedSize + identifier.size());

        char *identifierBuffer = writeBuffer;
        char *variableByteBuffer = writeBuffer + identifier.size();

        // for (size_t i = 0; i < identifier.size(); i++)
        // {
        //     ASSERT_EQ((uint8_t)identifierBuffer[i], (uint8_t)testData.identifier[i]) << "Buffers are incorrect at index " << i;
        // }

        for (size_t i = 0; i < data.encodedSize; i++)
        {
            ASSERT_EQ((uint8_t)variableByteBuffer[i], (uint8_t)data.encoded[i]) << "Buffers are incorrect at index " << i;
        }
    }

    delete variableByteProperty;
}

TYPED_TEST_P(VariableBytePropertyTest, Decode)
{
    VariableByteProperty *variableByteProperty = (VariableByteProperty *)new TypeParam();
    VariableByteInteger identifier = variableByteProperty->getIdentifier();

    for (auto &data : testData)
    {
        uint32_t variableBytesRead = 0;
        MockClient client;
        Client *clientPtr = (Client *)&client;
        uint32_t bytesRead = 0;
        char *readBuffer;

        variableByteProperty->setValue(VariableByteInteger());

        client.pushToReadBuffer((void *)data.encoded, data.encodedSize);

        variableByteProperty->readFromClient(clientPtr, variableBytesRead);

        ASSERT_EQ(variableBytesRead, data.encodedSize);

        ASSERT_EQ(data.raw, variableByteProperty->getValue().value);
    }

    delete variableByteProperty;
}

REGISTER_TYPED_TEST_SUITE_P(VariableBytePropertyTest, Encode, Decode);

using PropertyTypes = ::testing::Types<SubscriptionIdentifier>;

INSTANTIATE_TYPED_TEST_SUITE_P(My, VariableBytePropertyTest, PropertyTypes);
