/*
 * File: PicoMqttProperties.h
 * Project: cpp_mqtt_client
 * Created Date: Sunday February 26th 2023
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

#ifndef PICOMQTTPROPERTIES
#define PICOMQTTPROPERTIES

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "types/VariableByteInteger.h"
#include "types/EncodedString.h"
#include "Client.h"
#include "ClientInteractor.h"

#include "properties/Property.h"
#include "properties/ByteProperty.h"
#include "properties/DWordProperty.h"
#include "properties/StringPairProperty.h"
#include "properties/StringProperty.h"
#include "properties/VariableByteProperty.h"
#include "properties/WordProperty.h"

using namespace std;

namespace PicoMqtt
{

    /**
     * @brief
     *
     */
    class Properties : public ClientInteractor
    {
    private:
        uint32_t count = 0;
        vector<Property *> properties;
        uint8_t state = 0;

    protected:
    public:
        Properties();
        Properties(uint32_t count);
        ~Properties();
        size_t size();
        uint32_t length();
        void addProperty(Property *property);
        void clear();

        static Property *constructPropertyFromId(PropertyCodes identifier);

        bool readFromClient(Client *, uint32_t *);
        size_t pushToClient(Client *);
        size_t pushToBuffer(void *);
    };

    class PayloadFormatIndicatorProperty : public ByteProperty
    {
    public:
        PayloadFormatIndicatorProperty() : ByteProperty(PAYLOAD_FORMAT_INDICATOR){};
        PayloadFormatIndicatorProperty(uint8_t value) : ByteProperty(PAYLOAD_FORMAT_INDICATOR, value){};
    };

    class MessageExpiryIntervalProperty : public DWordProperty
    {
    public:
        MessageExpiryIntervalProperty() : DWordProperty(MESSAGE_EXPIRY_INTERVAL){};
        MessageExpiryIntervalProperty(uint32_t value) : DWordProperty(MESSAGE_EXPIRY_INTERVAL, value){};
    };

    class ContentTypeProperty : public StringProperty
    {
    public:
        ContentTypeProperty() : StringProperty(CONTENT_TYPE){};
        ContentTypeProperty(EncodedString value) : StringProperty(CONTENT_TYPE, value){};
        ContentTypeProperty(const char *value, uint32_t length) : StringProperty(CONTENT_TYPE, value, length){};
    };

    class ResponseTopic : public StringProperty
    {
    public:
        ResponseTopic() : StringProperty(RESPONSE_TOPIC){};
        ResponseTopic(EncodedString value) : StringProperty(RESPONSE_TOPIC, value){};
        ResponseTopic(const char *value, uint32_t length) : StringProperty(RESPONSE_TOPIC, value, length){};
    };

    class CorrelationData : public BinaryProperty
    {
    public:
        CorrelationData() : BinaryProperty(CORRELATION_DATA){};
        CorrelationData(EncodedString value) : BinaryProperty(CORRELATION_DATA, value){};
    };

    class SubscriptionIdentifier : public VariableByteProperty
    {
    public:
        SubscriptionIdentifier() : VariableByteProperty(SUBSCRIPTION_IDENTIFIER){};
        SubscriptionIdentifier(VariableByteInteger value) : VariableByteProperty(SUBSCRIPTION_IDENTIFIER, value){};
    };

    class SessionExpiryInterval : public DWordProperty
    {
    public:
        SessionExpiryInterval() : DWordProperty(SESSION_EXPIRY_INTERVAL){};
        SessionExpiryInterval(uint32_t value) : DWordProperty(SESSION_EXPIRY_INTERVAL, value){};
    };

    class AssignedClientIdentifier : public StringProperty
    {
    public:
        AssignedClientIdentifier() : StringProperty(ASSIGNED_CLIENT_IDENTIFER){};
        AssignedClientIdentifier(EncodedString value) : StringProperty(ASSIGNED_CLIENT_IDENTIFER, value){};
        AssignedClientIdentifier(const char *value, uint32_t length) : StringProperty(ASSIGNED_CLIENT_IDENTIFER, value, length){};
    };

    class ServerKeepAlive : public WordProperty
    {
    public:
        ServerKeepAlive() : WordProperty(SERVER_KEEP_ALIVE){};
        ServerKeepAlive(uint16_t value) : WordProperty(SERVER_KEEP_ALIVE, value){};
    };

    class AuthenticationMethod : public StringProperty
    {
    public:
        AuthenticationMethod() : StringProperty(AUTHENTICATION_METHOD){};
        AuthenticationMethod(EncodedString value) : StringProperty(AUTHENTICATION_METHOD, value){};
        AuthenticationMethod(const char *value, uint32_t length) : StringProperty(AUTHENTICATION_METHOD, value, length){};
    };

    class AuthenticationData : public BinaryProperty
    {
    public:
        AuthenticationData() : BinaryProperty(AUTHENTICATION_DATA){};
        AuthenticationData(EncodedString value) : BinaryProperty(AUTHENTICATION_DATA, value){};
        AuthenticationData(const char *value, uint32_t length) : BinaryProperty(AUTHENTICATION_METHOD, value, length){};
    };

    class RequestProblemInformation : public ByteProperty
    {
    public:
        RequestProblemInformation() : ByteProperty(REQUEST_PROBLEM_INFORMATION){};
        RequestProblemInformation(uint8_t value) : ByteProperty(REQUEST_PROBLEM_INFORMATION, value){};
    };

    class WillDelayInterval : public DWordProperty
    {
    public:
        WillDelayInterval() : DWordProperty(WILL_DELAY_INTERVAL){};
        WillDelayInterval(uint32_t value) : DWordProperty(WILL_DELAY_INTERVAL, value){};
    };

    class RequestResponseInformation : public ByteProperty
    {
    public:
        RequestResponseInformation() : ByteProperty(REQUEST_RESPONSE_INFORMATION){};
        RequestResponseInformation(uint8_t value) : ByteProperty(REQUEST_RESPONSE_INFORMATION, value){};
    };

    class ResponseInformation : public StringProperty
    {
    public:
        ResponseInformation() : StringProperty(RESPONSE_INFORMATION){};
        ResponseInformation(EncodedString value) : StringProperty(RESPONSE_INFORMATION, value){};
        ResponseInformation(const char *value, uint32_t length) : StringProperty(RESPONSE_INFORMATION, value, length){};
    };

    class ServerReference : public StringProperty
    {
    public:
        ServerReference() : StringProperty(SERVER_REFERENCE){};
        ServerReference(EncodedString value) : StringProperty(SERVER_REFERENCE, value){};
        ServerReference(const char *value, uint32_t length) : StringProperty(SERVER_REFERENCE, value, length){};
    };

    class ReasonString : public StringProperty
    {
    public:
        ReasonString() : StringProperty(REASON_STRING){};
        ReasonString(EncodedString value) : StringProperty(REASON_STRING, value){};
        ReasonString(const char *value, uint32_t length) : StringProperty(REASON_STRING, value, length){};
    };

    class ReceiveMaxium : public WordProperty
    {
    public:
        ReceiveMaxium() : WordProperty(RECEIVE_MAXIMUM){};
        ReceiveMaxium(uint16_t value) : WordProperty(RECEIVE_MAXIMUM, value){};
    };

    class TopicAliasMaximum : public WordProperty
    {
    public:
        TopicAliasMaximum() : WordProperty(TOPIC_ALIAS_MAXIMUM){};
        TopicAliasMaximum(uint16_t value) : WordProperty(TOPIC_ALIAS_MAXIMUM, value){};
    };

    class TopicAlias : public WordProperty
    {
    public:
        TopicAlias() : WordProperty(TOPIC_ALIAS){};
        TopicAlias(uint16_t value) : WordProperty(TOPIC_ALIAS, value){};
    };

    class MaxiumumQoS : public ByteProperty
    {
    public:
        MaxiumumQoS() : ByteProperty(MAXIMUM_QOS){};
        MaxiumumQoS(uint8_t value) : ByteProperty(MAXIMUM_QOS, value){};
    };

    class RetainAvailable : public ByteProperty
    {
    public:
        RetainAvailable() : ByteProperty(RETAIN_AVAILABLE){};
        RetainAvailable(uint8_t value) : ByteProperty(RETAIN_AVAILABLE, value){};
    };

    class UserProperty : public StringPairProperty
    {
    public:
        UserProperty() : StringPairProperty(USER_PROPERTY){};
        UserProperty(EncodedString key, EncodedString value) : StringPairProperty(USER_PROPERTY, key, value){};
    };

    class MaximumPacketSize : public DWordProperty
    {
    public:
        MaximumPacketSize() : DWordProperty(MAXIMUM_PACKET_SIZE){};
        MaximumPacketSize(uint32_t value) : DWordProperty(MAXIMUM_PACKET_SIZE, value){};
    };

    class WildcardSubscriptionAvailable : public ByteProperty
    {
    public:
        WildcardSubscriptionAvailable() : ByteProperty(WILDCARD_SUBSCRIPTION_AVAILABLE){};
        WildcardSubscriptionAvailable(uint8_t value) : ByteProperty(WILDCARD_SUBSCRIPTION_AVAILABLE, value){};
    };

    class SubscriptionIdentifierAvailable : public ByteProperty
    {
    public:
        SubscriptionIdentifierAvailable() : ByteProperty(SUBSCRIPTION_IDENTIFIERS_AVAILABLE){};
        SubscriptionIdentifierAvailable(uint8_t value) : ByteProperty(SUBSCRIPTION_IDENTIFIERS_AVAILABLE, value){};
    };

    class SharedSubscriptionAvailable : public ByteProperty
    {
    public:
        SharedSubscriptionAvailable() : ByteProperty(SHARED_SUBSCRIPTION_AVAILABLE){};
        SharedSubscriptionAvailable(uint8_t value) : ByteProperty(SHARED_SUBSCRIPTION_AVAILABLE, value){};
    };
}
#endif /* PICOMQTTPROPERTIES */
