#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"

#include "../mocks/MockClient.h"
#include "MqttProperties.h"

using namespace std;

using namespace CppMqtt;

struct WordValueTestSet
{
    uint32_t raw;
    const uint8_t *encoded;
    size_t encodedSize;
};

/* Must be in Big Endian format for MQTT 5 */
const uint8_t NONE_BUFFER[] = {0x0, 0x0};
const uint8_t ONE_MIN_BUFFER[] = {0x0, 0x7F};
const uint8_t ONE_MAX_BUFFER[] = {0x0, 0xFF};
const uint8_t TWO_MIN_BUFFER[] = {0xFF, 0x7F};
const uint8_t TWO_MAX_BUFFER[] = {0xFF, 0xFF};

static std::vector<WordValueTestSet> testData{
    {0, NONE_BUFFER, 2},
    {127, ONE_MIN_BUFFER, 2},
    {255, ONE_MAX_BUFFER, 2},
    {65407, TWO_MIN_BUFFER, 2},
    {65535, TWO_MAX_BUFFER, 2},
};

template <typename T>
class WordPropertyTest : public ::testing::Test
{
};

TYPED_TEST_SUITE_P(WordPropertyTest);

TYPED_TEST_P(WordPropertyTest, Encode)
{
    WordProperty *wordProperty = (WordProperty *)new TypeParam();
    VariableByteInteger identifier = wordProperty->getIdentifier();

    for (auto &data : testData)
    {
        MockClient client;
        Client *clientPtr = (Client *)&client;
        size_t bytesWritten;

        wordProperty->setValue(data.raw);
        PacketBuffer packetBuffer(wordProperty->size());
        bytesWritten = wordProperty->push(packetBuffer);

        char *writeBuffer = (char *)packetBuffer.getBuffer();

        ASSERT_EQ(bytesWritten, data.encodedSize + identifier.size());

        char *identifierBuffer = writeBuffer;
        char *wordBuffer = writeBuffer + identifier.size();

        // for (size_t i = 0; i < identifier.size(); i++)
        // {
        //     ASSERT_EQ((uint8_t)identifierBuffer[i], (uint8_t)testData.identifier[i]) << "Buffers are incorrect at index " << i;
        // }

        for (size_t i = 0; i < data.encodedSize; i++)
        {
            ASSERT_EQ((uint8_t)wordBuffer[i], (uint8_t)data.encoded[i]) << "Buffers are incorrect at index " << i;
        }
    }

    delete wordProperty;
}

TYPED_TEST_P(WordPropertyTest, Decode)
{
    WordProperty *wordProperty = (WordProperty *)new TypeParam();
    VariableByteInteger identifier = wordProperty->getIdentifier();

    for (auto &data : testData)
    {
        uint32_t wordsRead = 0;
        MockClient client;
        Client *clientPtr = (Client *)&client;
        uint32_t bytesRead = 0;
        char *readBuffer;

        client.resizeReadBuffer(data.encodedSize);
        readBuffer = client.getReadBuffer();
        memcpy(readBuffer, data.encoded, data.encodedSize);

        wordProperty->readFromClient(clientPtr, wordsRead);

        ASSERT_EQ(wordsRead, data.encodedSize);

        ASSERT_EQ(data.raw, wordProperty->getValue());
    }

    delete wordProperty;
}

REGISTER_TYPED_TEST_SUITE_P(WordPropertyTest, Encode, Decode);

using PropertyTypes = ::testing::Types<ServerKeepAlive, ReceiveMaxium, TopicAliasMaximum, TopicAlias>;

INSTANTIATE_TYPED_TEST_SUITE_P(My, WordPropertyTest, PropertyTypes);
