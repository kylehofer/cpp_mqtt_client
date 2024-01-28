#include "MqttTestHandler.h"
namespace PicoMqtt
{
    void MqttTestHandler::onConnectionSuccess()
    {
        connectionResult = 0;
    }

    void MqttTestHandler::onConnectionFailure(int reasonCode)
    {
        connectionResult = reasonCode;
    }

    void MqttTestHandler::onDisconnection(ReasonCode reasonCode)
    {
        disconnectionResult = reasonCode._to_integral();
    }

    void MqttTestHandler::onMessage(EncodedString &topic, Payload &payload)
    {
        topicQueue.push(topic);
        payloadQueue.push(payload);
    }

    void MqttTestHandler::onDeliveryComplete(Token token)
    {
        deliveryQueue.push(token);
    }

    void MqttTestHandler::onDeliveryFailure(Token token, int reasonCode)
    {
        deliveryFailureQueue.push({token, reasonCode});
    }

    void MqttTestHandler::onSubscribeResult(Token token, vector<uint8_t> reasonCodes)
    {
        subscribeResult[token] = reasonCodes;
    }

    void MqttTestHandler::onUnsubscribeResult(Token token, vector<uint8_t> reasonCodes)
    {
        unsubscribeResult[token] = reasonCodes;
    }

}