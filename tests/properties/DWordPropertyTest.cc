#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"

#include "../mocks/MockClient.h"
#include "PicoMqttProperties.h"

using namespace std;

using namespace PicoMqtt;

struct DWordValueTestSet
{
    uint32_t raw;
    const uint8_t *encoded;
    size_t encodedSize;
};

const uint8_t NONE_BUFFER[] = {0x0, 0x0, 0x0, 0x0};
const uint8_t ONE_MIN_BUFFER[] = {0x7F, 0x0, 0x0, 0x0};
const uint8_t ONE_MAX_BUFFER[] = {0xFF, 0x0, 0x0, 0x0};
const uint8_t TWO_MIN_BUFFER[] = {0x7F, 0xFF, 0x0, 0x0};
const uint8_t TWO_MAX_BUFFER[] = {0xFF, 0xFF, 0x0, 0x0};
const uint8_t THREE_MIN_BUFFER[] = {0x7F, 0xFF, 0xFF, 0x0};
const uint8_t THREE_MAX_BUFFER[] = {0xFF, 0xFF, 0xFF, 0x0};
const uint8_t FOUR_MIN_BUFFER[] = {0x7F, 0xFF, 0xFF, 0xFF};
const uint8_t FOUR_MAX_BUFFER[] = {0xFF, 0xFF, 0xFF, 0xFF};

static std::vector<DWordValueTestSet> testData{
    {0, NONE_BUFFER, 4},
    {127, ONE_MIN_BUFFER, 4},
    {255, ONE_MAX_BUFFER, 4},
    {65407, TWO_MIN_BUFFER, 4},
    {65535, TWO_MAX_BUFFER, 4},
    {16777087, THREE_MIN_BUFFER, 4},
    {16777215, THREE_MAX_BUFFER, 4},
    {4294967167, FOUR_MIN_BUFFER, 4},
    {4294967295, FOUR_MAX_BUFFER, 4},
};

template <typename T>
class DWordPropertyTest : public ::testing::Test
{
};

TYPED_TEST_SUITE_P(DWordPropertyTest);

TYPED_TEST_P(DWordPropertyTest, Encode)
{
    DWordProperty *dWordProperty = (DWordProperty *)new TypeParam();
    VariableByteInteger identifier = dWordProperty->getIdentifier();

    for (auto &data : testData)
    {
        MockClient client;
        Client *clientPtr = (Client *)&client;
        size_t bytesWritten;

        dWordProperty->setValue(data.raw);
        bytesWritten = dWordProperty->pushToClient(clientPtr);

        char *writeBuffer = client.getWriteBuffer();

        ASSERT_EQ(bytesWritten, data.encodedSize + identifier.size());

        char *identifierBuffer = writeBuffer;
        char *dWordBuffer = writeBuffer + identifier.size();

        // for (size_t i = 0; i < identifier.size(); i++)
        // {
        //     ASSERT_EQ((uint8_t)identifierBuffer[i], (uint8_t)testData.identifier[i]) << "Buffers are incorrect at index " << i;
        // }

        for (size_t i = 0; i < data.encodedSize; i++)
        {
            ASSERT_EQ((uint8_t)dWordBuffer[i], (uint8_t)data.encoded[i]) << "Buffers are incorrect at index " << i;
        }
    }
    delete dWordProperty;
}

TYPED_TEST_P(DWordPropertyTest, Decode)
{
    DWordProperty *dWordProperty = (DWordProperty *)new TypeParam();
    VariableByteInteger identifier = dWordProperty->getIdentifier();

    for (auto &data : testData)
    {
        uint32_t dWordsRead = 0;
        MockClient client;
        Client *clientPtr = (Client *)&client;
        uint32_t bytesRead = 0;
        char *readBuffer;

        client.resizeReadBuffer(data.encodedSize);
        readBuffer = client.getReadBuffer();
        memcpy(readBuffer, data.encoded, data.encodedSize);

        dWordProperty->readFromClient(clientPtr, &dWordsRead);

        ASSERT_EQ(dWordsRead, data.encodedSize);

        ASSERT_EQ(data.raw, dWordProperty->getValue());
    }

    delete dWordProperty;
}

REGISTER_TYPED_TEST_SUITE_P(DWordPropertyTest, Encode, Decode);

using PropertyTypes = ::testing::Types<MessageExpiryIntervalProperty, SessionExpiryInterval, WillDelayInterval, MaximumPacketSize>;

INSTANTIATE_TYPED_TEST_SUITE_P(My, DWordPropertyTest, PropertyTypes);
