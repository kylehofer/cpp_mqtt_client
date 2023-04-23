/*
 * File: MockMqttClient.h
 * Project: cpp_mqtt_client
 * Created Date: Sunday April 23rd 2023
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

#ifndef MOCKMQTTCLIENT
#define MOCKMQTTCLIENT

#include "MqttClient.h"
namespace PicoMqtt
{
    /**
     * @brief Mock Client for controlling time
     *
     */
    class MockMqttClient : public MqttClient
    {
    private:
        uint32_t elapsed = 0;

    protected:
        virtual uint32_t getElapsed() override
        {
            uint32_t value = elapsed;
            elapsed = 0;
            return value;
        }

    public:
        MockMqttClient() : MqttClient() {}
        MockMqttClient(Client *client) : MqttClient(client) {}

        void setElapsedTime(uint32_t value)
        {
            elapsed = value;
        }
    };
}

#endif /* MOCKMQTTCLIENT */
