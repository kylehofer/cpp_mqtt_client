#ifndef WILLPROPERTIES
#define WILLPROPERTIES

#include "PicoMqttProperties.h"
#include "types/EncodedString.h"
#include "types/BinaryData.h"

namespace PicoMqtt
{
    class WillProperties : public Properties
    {
    private:
        EncodedString topic;
        BinaryData payload;
        /**
         * @brief Set the Payload Format Indicator
         * 0 (0x00) Byte Indicates that the Will Message is unspecified bytes
         * 1 (0x01) Byte Indicates that the Will Message is UTF-8 Encoded Character Data.
         *
         * @param value
         */
        void setPayloadFormatIndicator(uint8_t value);

    protected:
    public:
        WillProperties();

        /**
         * @brief Set the Will Delay Interval
         * The Server delays publishing the Client’s Will Message until the Will Delay Interval has passed or the Session ends, whichever happens first.
         * If a new Network Connection to this Session is made before the Will Delay Interval has passed, the Server MUST NOT send the Will Message
         *
         * @param value
         */
        void setWillDelayInterval(uint32_t value);
        /**
         * @brief Set the Message Expiry Interval
         * The lifetime of the Will Message in seconds and is sent as the Publication Expiry Interval when the Server publishes the Will Message.
         *
         * @param value
         */
        void setMessageExpiryInterval(uint32_t value);
        /**
         * @brief Set the Content Type
         * UTF-8 Encoded String describing the content of the Will Message
         *
         * @param payload
         */
        void setContentType(EncodedString payload);
        /**
         * @brief Set the Response Topic object
         *
         * @param payload
         */
        void setResponseTopic(EncodedString payload);
        /**
         * @brief Set the Correlation Data
         * The Correlation Data is used by the sender of the Request Message to identify which request the Response Message is for when it is received.
         *
         * @param payload
         */
        void setCorrelationData(EncodedString payload);
        /**
         * @brief
         *
         * @param key
         * @param value
         */
        void addUserProperty(EncodedString key, EncodedString value);
        /**
         * @brief Set the Topic the Will will publish with
         *
         * @param payload
         */
        void setWillTopic(EncodedString payload);
        /**
         * @brief Set the Payload of the Will
         * Will set the flag in the will to be in a UTF-8 Format
         *
         * @param payload
         */
        void setPayload(EncodedString payload);
        /**
         * @brief Set the Payload of the Will
         * Will set the flag in the will to be in a Binary Format
         *
         * @param payload
         */
        void setPayload(BinaryData payload);

        /**
         * @brief Pushes the contents of the Will to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t push(PacketBuffer &buffer) override;
    };

}

#endif /* WILLPROPERTIES */
