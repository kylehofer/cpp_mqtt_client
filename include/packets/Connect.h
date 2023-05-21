/*
 * File: Connect.h
 * Project: cpp_mqtt_client
 * Created Date: Monday February 27th 2023
 * Author: Kyle Hofer
 *
 * MIT License
 *
 * Copyright (c) 2023 Kyle Hofer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * HISTORY:
 */

#ifndef CONNECT
#define CONNECT

#include <stdint.h>
#include "PropertiesPacket.h"
#include "properties/WillProperties.h"
#include "types/VariableByteInteger.h"
#include "types/EncodedString.h"
#include "types/BinaryData.h"

namespace PicoMqtt
{
#define CONNECT_FLAGS_SIZE 3
    typedef union
    {
        struct
        {
            unsigned char reserved : 1;
            unsigned char clean : 1;
            unsigned char will : 1;
            unsigned char willqos : 2;
            unsigned char willretain : 1;
            unsigned char password : 1;
            unsigned char username : 1;
            uint16_t keepAliveInterval = 0;
        };
        uint8_t data[CONNECT_FLAGS_SIZE];
    } ConnectFlags;

    /**
     * @brief Represents a MQTT 5 Connect Packet
     *
     */
    class Connect : public PropertiesPacket
    {
    private:
        uint8_t state = 0;
        ConnectFlags connectFlags;

        EncodedString clientIdentifier;
        WillProperties *willProperties = NULL;
        EncodedString *userName;
        BinaryData *password;

    protected:
    public:
        Connect();
        Connect(EncodedString id);
        size_t size();

        /**
         * @brief Pushes the contents of the Connect Packet to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t pushToClient(Client *client) override;
        bool readFromClient(Client *, uint32_t &) { return false; };

        // Properties
        void sessionExpiryInterval(uint32_t value);
        void setReceiveMaximum(uint32_t value);
        // Useful for embedded systems
        void setMaximumPacketSize(uint32_t value);
        // 0 or absent means no topic alias
        void setTopicAliasMaximum(uint16_t value);
        // Request Connect Acknowledge
        void setRequestResponseInformation(bool value);
        void setRequestProblemInformation(bool value);
        void addUserProperty(EncodedString key, EncodedString value);
        void setAuthenticationMethod(EncodedString value);

        void setWill(WillProperties *will);
        void setCleanStart(bool value);
        void setKeepAliveInterval(uint16_t value);
        uint16_t getKeepAliveInterval();

        void setClientId(EncodedString &id);
        void setClientId(const char *data, uint16_t length);
    };

}

#endif /* CONNECT */
