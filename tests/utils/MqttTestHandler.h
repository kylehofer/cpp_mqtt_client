#ifndef MQTTTESTCLIENT
#define MQTTTESTCLIENT

#include "MqttClient.h"
#include <queue>
#include <tuple>
#include <map>

namespace PicoMqtt
{

    class MqttTestHandler : MqttClientHandler
    {
    public:
        int connectionResult = -1;
        int disconnectionResult = -1;
        map<Token, vector<uint8_t>> subscribeResult;
        map<Token, vector<uint8_t>> unsubscribeResult;
        queue<EncodedString> topicQueue;
        queue<Payload> payloadQueue;
        queue<Token> deliveryQueue;
        queue<tuple<Token, uint8_t>> deliveryFailureQueue;

        virtual void onConnectionSuccess() override;
        virtual void onConnectionFailure(int reasonCode) override;
        virtual void onDisconnection(int reasonCode) override;
        virtual void onMessage(EncodedString &topic, Payload &payload) override;
        virtual void onDeliveryComplete(Token token) override;
        virtual void onDeliveryFailure(Token token, int reasonCode) override;
        virtual void onSubscribeResult(Token token, vector<uint8_t> reasonCodes) override;
        virtual void onUnsubscribeResult(Token token, vector<uint8_t> reasonCodes) override;
    };
}

#endif /* MQTTTESTCLIENT */
