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
        void setPayloadFormatIndicator(uint8_t byte);

    protected:
    public:
        WillProperties();

        using Properties::push;

        void setWillDelayInterval(uint32_t value);
        void setMessageExpiryInterval(uint32_t value);
        void setContentType(EncodedString payload);
        void setResponseTopic(EncodedString payload);
        void setCorrelationData(EncodedString payload);
        void addUserProperty(EncodedString key, EncodedString value);

        void setWillTopic(EncodedString payload);
        void setPayload(EncodedString payload);
        void setPayload(BinaryData payload);
    };

}

#endif /* WILLPROPERTIES */
