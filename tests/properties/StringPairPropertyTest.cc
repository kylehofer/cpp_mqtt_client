#include <iostream>
#include <vector>
#include "gtest/gtest.h"
#include "stdint.h"
#include <string>

#include "../mocks/MockClient.h"
#include "PicoMqttProperties.h"

using namespace std;

using namespace PicoMqtt;

struct StringPairTestData
{
    const char *rawKey;
    size_t rawKeySize;
    const char *rawValue;
    size_t rawValueSize;
    const uint8_t *encoded;
    size_t encodedSize;
};

const uint8_t NULL_NULL_BUFFER[] = {0x0, 0x0, 0x0, 0x0};
const uint8_t SIXTEEN_SIXTEEN_BUFFER[] = {0x0, 0x10, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x31, 0x36, 0x20, 0x43, 0x68, 0x61, 0x72, 0x0, 0x0, 0x10, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x31, 0x36, 0x20, 0x43, 0x68, 0x61, 0x72, 0x0};
const uint8_t SIXTEEN_LOREM_BUFFER[] = {0x0, 0x10, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x31, 0x36, 0x20, 0x43, 0x68, 0x61, 0x72, 0x0, 0x01, 0x0F, 0x4C, 0x6F, 0x72, 0x65, 0x6D, 0x20, 0x69, 0x70, 0x73, 0x75, 0x6D, 0x20, 0x64, 0x6F, 0x6C, 0x6F, 0x72, 0x20, 0x73, 0x69, 0x74, 0x20, 0x61, 0x6D, 0x65, 0x74, 0x2C, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x65, 0x63, 0x74, 0x65, 0x74, 0x75, 0x72, 0x20, 0x61, 0x64, 0x69, 0x70, 0x69, 0x73, 0x63, 0x69, 0x6E, 0x67, 0x20, 0x65, 0x6C, 0x69, 0x74, 0x2E, 0x20, 0x50, 0x72, 0x61, 0x65, 0x73, 0x65, 0x6E, 0x74, 0x20, 0x75, 0x74, 0x20, 0x62, 0x69, 0x62, 0x65, 0x6E, 0x64, 0x75, 0x6D, 0x20, 0x72, 0x69, 0x73, 0x75, 0x73, 0x2E, 0x20, 0x4E, 0x75, 0x6C, 0x6C, 0x61, 0x6D, 0x20, 0x76, 0x65, 0x6E, 0x65, 0x6E, 0x61, 0x74, 0x69, 0x73, 0x20, 0x65, 0x72, 0x61, 0x74, 0x20, 0x74, 0x65, 0x6D, 0x70, 0x6F, 0x72, 0x20, 0x61, 0x75, 0x67, 0x75, 0x65, 0x20, 0x63, 0x6F, 0x6E, 0x76, 0x61, 0x6C, 0x6C, 0x69, 0x73, 0x2C, 0x20, 0x76, 0x69, 0x74, 0x61, 0x65, 0x20, 0x73, 0x65, 0x6D, 0x70, 0x65, 0x72, 0x20, 0x6C, 0x65, 0x6F, 0x20, 0x61, 0x75, 0x63, 0x74, 0x6F, 0x72, 0x2E, 0x20, 0x50, 0x72, 0x6F, 0x69, 0x6E, 0x20, 0x6F, 0x72, 0x6E, 0x61, 0x72, 0x65, 0x20, 0x65, 0x72, 0x61, 0x74, 0x20, 0x61, 0x63, 0x20, 0x74, 0x6F, 0x72, 0x74, 0x6F, 0x72, 0x20, 0x62, 0x6C, 0x61, 0x6E, 0x64, 0x69, 0x74, 0x2C, 0x20, 0x61, 0x20, 0x64, 0x69, 0x63, 0x74, 0x75, 0x6D, 0x20, 0x6C, 0x69, 0x67, 0x75, 0x6C, 0x61, 0x20, 0x76, 0x61, 0x72, 0x69, 0x75, 0x73, 0x2E, 0x20, 0x50, 0x72, 0x6F, 0x69, 0x6E, 0x20, 0x6E, 0x65, 0x63, 0x20, 0x6E, 0x69, 0x62, 0x68, 0x20, 0x73, 0x6F, 0x6C, 0x6C, 0x69, 0x63, 0x69, 0x74, 0x75, 0x64, 0x69, 0x6E, 0x2C, 0x20, 0x66, 0x61, 0x75, 0x63, 0x69, 0x62, 0x75, 0x73, 0x20, 0x64, 0x75, 0x69, 0x20, 0x76, 0x65, 0x6C, 0x20, 0x6C, 0x65, 0x63, 0x74, 0x75, 0x73, 0x2E, 0x0};
const uint8_t LOREM_SIXTEEN_BUFFER[] = {0x01, 0x0F, 0x4C, 0x6F, 0x72, 0x65, 0x6D, 0x20, 0x69, 0x70, 0x73, 0x75, 0x6D, 0x20, 0x64, 0x6F, 0x6C, 0x6F, 0x72, 0x20, 0x73, 0x69, 0x74, 0x20, 0x61, 0x6D, 0x65, 0x74, 0x2C, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x65, 0x63, 0x74, 0x65, 0x74, 0x75, 0x72, 0x20, 0x61, 0x64, 0x69, 0x70, 0x69, 0x73, 0x63, 0x69, 0x6E, 0x67, 0x20, 0x65, 0x6C, 0x69, 0x74, 0x2E, 0x20, 0x50, 0x72, 0x61, 0x65, 0x73, 0x65, 0x6E, 0x74, 0x20, 0x75, 0x74, 0x20, 0x62, 0x69, 0x62, 0x65, 0x6E, 0x64, 0x75, 0x6D, 0x20, 0x72, 0x69, 0x73, 0x75, 0x73, 0x2E, 0x20, 0x4E, 0x75, 0x6C, 0x6C, 0x61, 0x6D, 0x20, 0x76, 0x65, 0x6E, 0x65, 0x6E, 0x61, 0x74, 0x69, 0x73, 0x20, 0x65, 0x72, 0x61, 0x74, 0x20, 0x74, 0x65, 0x6D, 0x70, 0x6F, 0x72, 0x20, 0x61, 0x75, 0x67, 0x75, 0x65, 0x20, 0x63, 0x6F, 0x6E, 0x76, 0x61, 0x6C, 0x6C, 0x69, 0x73, 0x2C, 0x20, 0x76, 0x69, 0x74, 0x61, 0x65, 0x20, 0x73, 0x65, 0x6D, 0x70, 0x65, 0x72, 0x20, 0x6C, 0x65, 0x6F, 0x20, 0x61, 0x75, 0x63, 0x74, 0x6F, 0x72, 0x2E, 0x20, 0x50, 0x72, 0x6F, 0x69, 0x6E, 0x20, 0x6F, 0x72, 0x6E, 0x61, 0x72, 0x65, 0x20, 0x65, 0x72, 0x61, 0x74, 0x20, 0x61, 0x63, 0x20, 0x74, 0x6F, 0x72, 0x74, 0x6F, 0x72, 0x20, 0x62, 0x6C, 0x61, 0x6E, 0x64, 0x69, 0x74, 0x2C, 0x20, 0x61, 0x20, 0x64, 0x69, 0x63, 0x74, 0x75, 0x6D, 0x20, 0x6C, 0x69, 0x67, 0x75, 0x6C, 0x61, 0x20, 0x76, 0x61, 0x72, 0x69, 0x75, 0x73, 0x2E, 0x20, 0x50, 0x72, 0x6F, 0x69, 0x6E, 0x20, 0x6E, 0x65, 0x63, 0x20, 0x6E, 0x69, 0x62, 0x68, 0x20, 0x73, 0x6F, 0x6C, 0x6C, 0x69, 0x63, 0x69, 0x74, 0x75, 0x64, 0x69, 0x6E, 0x2C, 0x20, 0x66, 0x61, 0x75, 0x63, 0x69, 0x62, 0x75, 0x73, 0x20, 0x64, 0x75, 0x69, 0x20, 0x76, 0x65, 0x6C, 0x20, 0x6C, 0x65, 0x63, 0x74, 0x75, 0x73, 0x2E, 0x0, 0x0, 0x10, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x31, 0x36, 0x20, 0x43, 0x68, 0x61, 0x72, 0x0};
const uint8_t LOREM_BUFFER[] = {0x01, 0x0F, 0x4C, 0x6F, 0x72, 0x65, 0x6D, 0x20, 0x69, 0x70, 0x73, 0x75, 0x6D, 0x20, 0x64, 0x6F, 0x6C, 0x6F, 0x72, 0x20, 0x73, 0x69, 0x74, 0x20, 0x61, 0x6D, 0x65, 0x74, 0x2C, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x65, 0x63, 0x74, 0x65, 0x74, 0x75, 0x72, 0x20, 0x61, 0x64, 0x69, 0x70, 0x69, 0x73, 0x63, 0x69, 0x6E, 0x67, 0x20, 0x65, 0x6C, 0x69, 0x74, 0x2E, 0x20, 0x50, 0x72, 0x61, 0x65, 0x73, 0x65, 0x6E, 0x74, 0x20, 0x75, 0x74, 0x20, 0x62, 0x69, 0x62, 0x65, 0x6E, 0x64, 0x75, 0x6D, 0x20, 0x72, 0x69, 0x73, 0x75, 0x73, 0x2E, 0x20, 0x4E, 0x75, 0x6C, 0x6C, 0x61, 0x6D, 0x20, 0x76, 0x65, 0x6E, 0x65, 0x6E, 0x61, 0x74, 0x69, 0x73, 0x20, 0x65, 0x72, 0x61, 0x74, 0x20, 0x74, 0x65, 0x6D, 0x70, 0x6F, 0x72, 0x20, 0x61, 0x75, 0x67, 0x75, 0x65, 0x20, 0x63, 0x6F, 0x6E, 0x76, 0x61, 0x6C, 0x6C, 0x69, 0x73, 0x2C, 0x20, 0x76, 0x69, 0x74, 0x61, 0x65, 0x20, 0x73, 0x65, 0x6D, 0x70, 0x65, 0x72, 0x20, 0x6C, 0x65, 0x6F, 0x20, 0x61, 0x75, 0x63, 0x74, 0x6F, 0x72, 0x2E, 0x20, 0x50, 0x72, 0x6F, 0x69, 0x6E, 0x20, 0x6F, 0x72, 0x6E, 0x61, 0x72, 0x65, 0x20, 0x65, 0x72, 0x61, 0x74, 0x20, 0x61, 0x63, 0x20, 0x74, 0x6F, 0x72, 0x74, 0x6F, 0x72, 0x20, 0x62, 0x6C, 0x61, 0x6E, 0x64, 0x69, 0x74, 0x2C, 0x20, 0x61, 0x20, 0x64, 0x69, 0x63, 0x74, 0x75, 0x6D, 0x20, 0x6C, 0x69, 0x67, 0x75, 0x6C, 0x61, 0x20, 0x76, 0x61, 0x72, 0x69, 0x75, 0x73, 0x2E, 0x20, 0x50, 0x72, 0x6F, 0x69, 0x6E, 0x20, 0x6E, 0x65, 0x63, 0x20, 0x6E, 0x69, 0x62, 0x68, 0x20, 0x73, 0x6F, 0x6C, 0x6C, 0x69, 0x63, 0x69, 0x74, 0x75, 0x64, 0x69, 0x6E, 0x2C, 0x20, 0x66, 0x61, 0x75, 0x63, 0x69, 0x62, 0x75, 0x73, 0x20, 0x64, 0x75, 0x69, 0x20, 0x76, 0x65, 0x6C, 0x20, 0x6C, 0x65, 0x63, 0x74, 0x75, 0x73, 0x2E, 0x0};

static vector<StringPairTestData> testData({
    {"This is 16 Char",
     16,
     "This is 16 Char",
     16,
     SIXTEEN_SIXTEEN_BUFFER,
     36},
    {"This is 16 Char",
     16,
     "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent ut bibendum risus. Nullam venenatis erat tempor augue convallis, vitae semper leo auctor. Proin ornare erat ac tortor blandit, a dictum ligula varius. Proin nec nibh sollicitudin, faucibus dui vel lectus.",
     271,
     SIXTEEN_LOREM_BUFFER,
     291},
    {"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent ut bibendum risus. Nullam venenatis erat tempor augue convallis, vitae semper leo auctor. Proin ornare erat ac tortor blandit, a dictum ligula varius. Proin nec nibh sollicitudin, faucibus dui vel lectus.",
     271,
     "This is 16 Char",
     16,
     LOREM_SIXTEEN_BUFFER,
     291},
    {NULL,
     0,
     NULL,
     0,
     NULL_NULL_BUFFER,
     4},
});

template <typename T>
class StringPairPropertyTest : public ::testing::Test
{
private:
protected:
    static void SetUpTestSuite()
    {
    }

    static void TearDownTestSuite()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TYPED_TEST_SUITE_P(StringPairPropertyTest);

TYPED_TEST_P(StringPairPropertyTest, Encode)
{
    StringPairProperty *stringPairProperty = (StringPairProperty *)new TypeParam();
    VariableByteInteger identifier = stringPairProperty->getIdentifier();

    for (auto &data : testData)
    {
        MockClient client;
        Client *clientPtr = (Client *)&client;

        EncodedString encodedKey(data.rawKey, data.rawKeySize);
        EncodedString encodedValue(data.rawValue, data.rawValueSize);
        size_t bytesWritten = 0;

        stringPairProperty->setKey(encodedKey);
        stringPairProperty->setValue(encodedValue);
        bytesWritten = stringPairProperty->pushToClient(clientPtr);

        char *writeBuffer = client.getWriteBuffer();

        ASSERT_EQ(bytesWritten, data.encodedSize + identifier.size());

        char *identifierBuffer = writeBuffer;
        char *stringBuffer = writeBuffer + identifier.size();

        // for (size_t i = 0; i < identifier.size(); i++)
        // {
        //     ASSERT_EQ(identifierBuffer[i], testData.identifier[i]) << "Buffers are incorrect at index " << i;
        // }

        for (size_t i = 0; i < data.encodedSize; i++)
        {
            ASSERT_EQ(stringBuffer[i], data.encoded[i]) << "Buffers are incorrect at index " << i;
        }
    }

    delete stringPairProperty;
}

TYPED_TEST_P(StringPairPropertyTest, Decode)
{
    StringPairProperty *stringPairProperty = (StringPairProperty *)new TypeParam();
    VariableByteInteger identifier = stringPairProperty->getIdentifier();

    for (auto &data : testData)
    {
        MockClient client;
        Client *clientPtr = (Client *)&client;

        uint32_t bytesRead = 0;
        char *readBuffer;

        client.pushToReadBuffer((void *)data.encoded, data.encodedSize);

        stringPairProperty->readFromClient(clientPtr, &bytesRead);

        ASSERT_EQ(bytesRead, data.encodedSize);

        EncodedString encodedKey = stringPairProperty->getKey();
        EncodedString encodedValue = stringPairProperty->getValue();

        char *keyBuffer = encodedKey.data;
        char *valueBuffer = encodedValue.data;

        for (size_t i = 0; i < data.rawKeySize; i++)
        {
            ASSERT_EQ(keyBuffer[i], data.rawKey[i]) << "Buffers are incorrect at index " << i;
        }

        for (size_t i = 0; i < data.rawValueSize; i++)
        {
            ASSERT_EQ(valueBuffer[i], data.rawValue[i]) << "Buffers are incorrect at index " << i;
        }
    }

    delete stringPairProperty;
}

REGISTER_TYPED_TEST_SUITE_P(StringPairPropertyTest, Encode, Decode);
using PropertyTypes = ::testing::Types<UserProperty>;
INSTANTIATE_TYPED_TEST_SUITE_P(My, StringPairPropertyTest, PropertyTypes);
