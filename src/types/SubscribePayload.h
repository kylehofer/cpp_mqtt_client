#ifndef SUBSCRIBEPAYLOAD
#define SUBSCRIBEPAYLOAD

#include <stdint.h>
#include "types/SubscriptionPayload.h"
// #include "types/EncodedString.h"
// #include "types/Common.h"
// #include "types/VariableByteInteger.h"

namespace PicoMqtt
{
    typedef union
    {
        struct
        {
            unsigned char reserved : 2;
            unsigned char retainHandling : 2;
            unsigned char retainAsPublished : 1;
            unsigned char noLocal : 1;
            unsigned char maximumQos : 2;
        };
        uint8_t data;
    } SubscriptionOptions;

    /**
     * @brief Represents a MQTT 5 Subscription Payload
     * Used for representing a single topic with a set of options for Un/subscribing.
     * Can read and write to a communication client
     */
    class SubscribePayload : public SubscriptionPayload
    {
    private:
        SubscriptionOptions options;

    protected:
    public:
        SubscribePayload();
        /**
         * @brief Pushes the contents of the Subscription Payload to a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t push(PacketBuffer &buffer) override;
        virtual size_t size() override;

        void setMaximumQos(QoS qos);
        QoS getMaximumQos();

        void setNoLocal(bool value);
        bool getNoLocal();

        void setRetainAsPublished(bool value);
        bool getRetainAsPublished();

        void setRetainHandling();
        void getRetainHandling();
    };

}

#endif /* SUBSCRIBEPAYLOAD */
