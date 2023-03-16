#include <iostream>
// #include <string>
#include <tuple>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"

#include "../mocks/MockClient.h"
#include "types/VariableByteInteger.h"

struct VariableTestSet
{
    uint32_t raw;
    uint32_t expected;
    uint32_t length;
};

std::vector<VariableTestSet> variableTestSet{

    {0, 0, 1},
    {1, 1, 1},
    {127, 127, 1},
    {384, 128, 2},
    {32767, 16383, 2},
    {98432, 16384, 3},
    {8388607, 2097151, 3},
    {25198720, 2097152, 4},
    {2147483647, 268435455, 4},
    // {2155905152, 268435455, 4}, // TODO Add Validation
    // {4294967295, 268435455, 4},
};

using namespace PicoMqtt;

class VariableByteIntegerTest : public ::testing::TestWithParam<VariableTestSet>
{
};

TEST_P(VariableByteIntegerTest, EncodeDecode)
{
    uint32_t bytesRead = 0;
    MockClient client;
    VariableByteInteger decodeInteger;

    Client *clientPtr = (Client *)&client;

    client.resizeReadBuffer(4);
    client.resizeWriteBuffer(4);

    char *readBuffer = client.getReadBuffer();
    char *writeBuffer = client.getWriteBuffer();

    VariableTestSet testData = GetParam();

    memcpy(readBuffer, &testData.raw, testData.length);

    decodeInteger.readFromClient(clientPtr, &bytesRead);

    EXPECT_EQ(decodeInteger.value, testData.expected);
    EXPECT_EQ(decodeInteger.size(), testData.length);

    uint32_t length = decodeInteger.pushToClient(clientPtr);

    EXPECT_EQ(length, testData.length);

    for (int i = 0; i < length; i++)
    {
        EXPECT_EQ(readBuffer[i], writeBuffer[i]) << "Buffers are incorrect at index " << i;
    }
}

INSTANTIATE_TEST_CASE_P(AllCombinations,
                        VariableByteIntegerTest,
                        ::testing::ValuesIn(variableTestSet));