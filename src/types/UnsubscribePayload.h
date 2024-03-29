#ifndef UNSUBSCRIBEPAYLOAD
#define UNSUBSCRIBEPAYLOAD

#include "types/SubscriptionPayload.h"

namespace CppMqtt
{
    /**
     * @brief Represents a MQTT 5 Subscription Payload
     * Used for representing a single topic with a set of options for Un/subscribing.
     * Can read and write to a communication client
     */
    class UnsubscribePayload : public SubscriptionPayload
    {
    private:
    protected:
    public:
        UnsubscribePayload();
    };

}

#endif /* UNSUBSCRIBEPAYLOAD */
