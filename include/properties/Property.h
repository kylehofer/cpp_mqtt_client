/*
 * File: Property.h
 * Project: cpp_mqtt_client
 * Created Date: Tuesday March 7th 2023
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

#ifndef PROPERTY
#define PROPERTY

#include <stdint.h>
#include <stdio.h>
#include "ClientInteractor.h"
#include "types/VariableByteInteger.h"

/** The one byte  V5 property indicator */
enum PropertyCodes
{
    PAYLOAD_FORMAT_INDICATOR = 1,            /**< The value is 1 */
    MESSAGE_EXPIRY_INTERVAL = 2,             /**< The value is 2 */
    CONTENT_TYPE = 3,                        /**< The value is 3 */
    RESPONSE_TOPIC = 8,                      /**< The value is 8 */
    CORRELATION_DATA = 9,                    /**< The value is 9 */
    SUBSCRIPTION_IDENTIFIER = 11,            /**< The value is 11 */
    SESSION_EXPIRY_INTERVAL = 17,            /**< The value is 17 */
    ASSIGNED_CLIENT_IDENTIFER = 18,          /**< The value is 18 */
    SERVER_KEEP_ALIVE = 19,                  /**< The value is 19 */
    AUTHENTICATION_METHOD = 21,              /**< The value is 21 */
    AUTHENTICATION_DATA = 22,                /**< The value is 22 */
    REQUEST_PROBLEM_INFORMATION = 23,        /**< The value is 23 */
    WILL_DELAY_INTERVAL = 24,                /**< The value is 24 */
    REQUEST_RESPONSE_INFORMATION = 25,       /**< The value is 25 */
    RESPONSE_INFORMATION = 26,               /**< The value is 26 */
    SERVER_REFERENCE = 28,                   /**< The value is 28 */
    REASON_STRING = 31,                      /**< The value is 31 */
    RECEIVE_MAXIMUM = 33,                    /**< The value is 33*/
    TOPIC_ALIAS_MAXIMUM = 34,                /**< The value is 34 */
    TOPIC_ALIAS = 35,                        /**< The value is 35 */
    MAXIMUM_QOS = 36,                        /**< The value is 36 */
    RETAIN_AVAILABLE = 37,                   /**< The value is 37 */
    USER_PROPERTY = 38,                      /**< The value is 38 */
    MAXIMUM_PACKET_SIZE = 39,                /**< The value is 39 */
    WILDCARD_SUBSCRIPTION_AVAILABLE = 40,    /**< The value is 40 */
    SUBSCRIPTION_IDENTIFIERS_AVAILABLE = 41, /**< The value is 41 */
    SHARED_SUBSCRIPTION_AVAILABLE = 241      /**< The value is 241 */
};

namespace PicoMqtt
{
    /**
     * @brief
     *
     */
    class Property : public ClientInteractor
    {
    private:
        VariableByteInteger identifier;

    protected:
        /**
         * @brief Pushes the raw data of the Property to  a communications client
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        virtual size_t pushPropertyToClient(Client *client) = 0;
        /**
         * @brief Returns the byte size of the properties value
         *
         * @return size_t
         */
        virtual size_t propertySize() = 0;

    public:
        Property(){};
        Property(PropertyCodes identifier)
        {
            this->identifier.value = identifier;
        };
        virtual ~Property(){};

        VariableByteInteger getIdentifier()
        {
            return identifier;
        }

        size_t size()
        {
            return this->identifier.size() + propertySize();
        }

        /**
         * @brief Pushes the contents of the Property to a communications client
         * It will write the Property identifier, followed by the raw data of the Property
         *
         * @param client The client to push data to
         * @return size_t The amount of bytes written
         */
        size_t pushToClient(Client *client)
        {
            size_t identifierLength = this->identifier.pushToClient(client);
            size_t propertyLength = pushPropertyToClient(client);
            return identifierLength + propertyLength;
        };
    };
}
#endif /* PROPERTY */
