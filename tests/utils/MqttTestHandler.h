#ifndef MQTTTESTCLIENT
#define MQTTTESTCLIENT

#include "MqttClient.h"
#include <queue>
#include <tuple>
#include <map>

namespace CppMqtt
{

    class MqttTestHandler : MqttClientHandler
    {
    public:
        int connectionResult = -1;
        int disconnectionResult = -1;
        std::map<Token, ReasonCodes> subscribeResult;
        std::map<Token, ReasonCodes> unsubscribeResult;
        std::queue<EncodedString> topicQueue;
        std::queue<Payload> payloadQueue;
        std::queue<Token> deliveryQueue;
        std::queue<std::tuple<Token, uint8_t>> deliveryFailureQueue;

        virtual void onConnectionSuccess() override;
        virtual void onConnectionFailure(int reasonCode) override;
        virtual void onDisconnection(ReasonCode reasonCode) override;
        virtual void onMessage(EncodedString &topic, Payload &payload) override;
        virtual void onDeliveryComplete(Token token) override;
        virtual void onDeliveryFailure(Token token, int reasonCode) override;
        virtual void onSubscribeResult(Token token, ReasonCodes reasonCodes) override;
        virtual void onUnsubscribeResult(Token token, ReasonCodes reasonCodes) override;
    };
}

#endif /* MQTTTESTCLIENT */
