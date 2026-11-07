#include "gtest/gtest.h"
#include "utils/TestPacketBuffer.h"
#include "stdint.h"
#include <string>

#include "../mocks/MockClient.h"
#include "types/EncodedString.h"
#include "packets/Authentication.h"
#include "PacketBuffer.h"

using namespace std;
using namespace CppMqtt;

#define LENGTH_SIZE sizeof(uint16_t)

TEST(AuthenticationTest, push)
{
    MockClient client;
    Client *clientPtr = (Client *)&client;
    Authentication authentication;

    TestPacketBuffer packetBuffer;

    authentication.push(packetBuffer);

    uint8_t *buffer = packetBuffer.getBuffer();
    size_t length = packetBuffer.getLength();
}

TEST(AuthenticationTest, readFromClient)
{
    MockClient client;
    Client *clientPtr = (Client *)&client;
}