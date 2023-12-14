/*
 * File: UnsubscribeAcknowledge.h
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

#ifndef UNSUBSCRIBEACKNOWLEDGE
#define UNSUBSCRIBEACKNOWLEDGE

#include "ReasonsAcknowledge.h"
namespace PicoMqtt
{
    /**
     * @brief Represents a MQTT 5 Unsubscribe Acknowledge Packet
     * Contains a collection of reason codes for the results of the topic being unsubscribed
     */
    class UnsubscribeAcknowledge : public ReasonsAcknowledge
    {
    public:
        UnsubscribeAcknowledge();
        UnsubscribeAcknowledge(uint8_t flags);
        /**
         * @brief Validates the packet to the MQTT 5 standards
         *
         * @return true
         * @return false
         */
        virtual bool validate() override;
    };
}

#endif /* UNSUBSCRIBEACKNOWLEDGE */
