#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"
#include <string>

#include "../mocks/MockClient.h"
#include "types/EncodedString.h"

using namespace std;

struct EncodedTestSet
{
    const char *input;
    uint16_t inputSize;
    const char *expected;
    uint16_t expectedSize;
    uint8_t sizeRaw[2];
};

using namespace PicoMqtt;

class EncodedStringTest : public ::testing::TestWithParam<EncodedTestSet>
{
};

#define LENGTH_SIZE sizeof(uint16_t)

TEST(EncodedStringTest, EncodeDecode)
{
    vector<EncodedTestSet> encodedTestSet{

        {"This is 16 Char", 16, "This is 16 Char", 16, {0x0, 0x10}},
        {"four", 5, "four", 4, {0x0, 0x4}},
        {"two", 4, "tw", 2, {0x0, 0x2}},
        {"Long sentence with small", 25, "Long sente", 10, {0x0, 0xA}},
        {"", 1, "", 0, {0x0, 0x0}},
        {"should be ignored", 18, "", 0, {0x0, 0x0}},
    };

    for (auto &testData : encodedTestSet)
    {
        uint32_t bytesRead = 0;
        MockClient client;
        EncodedString decodedString;

        Client *clientPtr = (Client *)&client;

        size_t stringSize = testData.inputSize;

        client.resizeReadBuffer(stringSize + LENGTH_SIZE);
        client.resizeWriteBuffer(testData.expectedSize + LENGTH_SIZE);

        char *readBuffer = client.getReadBuffer();

        memcpy(readBuffer, testData.sizeRaw, LENGTH_SIZE);
        memcpy(readBuffer + LENGTH_SIZE, testData.input, stringSize);

        decodedString.readFromClient(clientPtr, bytesRead);

        ASSERT_EQ(decodedString.size(), testData.expectedSize + LENGTH_SIZE);

        for (int i = 0; i < testData.expectedSize; i++)
        {
            ASSERT_EQ(decodedString.data[i], testData.expected[i]) << "Buffers are incorrect at index " << i;
        }

        PacketBuffer packetBuffer(decodedString.size());
        uint32_t length = decodedString.push(packetBuffer);

        char *writeBuffer = (char *)packetBuffer.getBuffer();

        ASSERT_EQ(length, testData.expectedSize + LENGTH_SIZE);

        for (int i = 0; i < length; i++)
        {
            ASSERT_EQ(readBuffer[i], writeBuffer[i]) << "Buffers are incorrect at index " << i;
        }
    };
}

#define LARGE_SIZE 49

TEST(EncodedStringTest, PartialDecoding)
{
    uint32_t bytesRead = 0;
    MockClient client;
    EncodedString decodedString;

    Client *clientPtr = (Client *)&client;

    // Push using Big Endian ordering
    uint8_t byte = 0;

    client.pushToReadBuffer(&byte, 1);

    decodedString.readFromClient(clientPtr, bytesRead);

    EXPECT_EQ(decodedString.size(), 2);
    EXPECT_EQ(decodedString.data, nullptr);

    byte = LARGE_SIZE;

    client.pushToReadBuffer(&byte, 1);

    EXPECT_EQ(decodedString.size(), LENGTH_SIZE);
    EXPECT_EQ(decodedString.data, nullptr);

    decodedString.readFromClient(clientPtr, bytesRead);

    EXPECT_EQ(decodedString.size(), 51);
    EXPECT_NE(decodedString.data, nullptr);

    decodedString.readFromClient(clientPtr, bytesRead);

    ASSERT_EQ(decodedString.size(), 51);
    EXPECT_NE(decodedString.data, nullptr);

    for (size_t i = 0; i < LARGE_SIZE; i++)
    {
        ASSERT_EQ(decodedString.data[i], '\0');
    }

    client.pushToReadBuffer((void *)"A fairly large string but not large enough", 42);

    decodedString.readFromClient(clientPtr, bytesRead);

    ASSERT_EQ(decodedString.size(), 51);
    ASSERT_NE(decodedString.data, nullptr);

    for (size_t i = 0; i < LARGE_SIZE; i++)
    {
        ASSERT_EQ(decodedString.data[i], '\0');
    }

    client.pushToReadBuffer((void *)", but this should finish off the string", 40);

    decodedString.readFromClient(clientPtr, bytesRead);

    ASSERT_EQ(decodedString.size(), 51);
    ASSERT_NE(decodedString.data, nullptr);

    const char *testValue = "A fairly large string but not large enough, but thi";

    for (size_t i = 0; i < LARGE_SIZE; i++)
    {
        ASSERT_EQ(decodedString.data[i], testValue[i]) << "at position " << i;
    }
}

// INSTANTIATE_TEST_CASE_P(AllCombinations,
//                         EncodedStringTest,
//                         ::testing::ValuesIn(encodedTestSet));