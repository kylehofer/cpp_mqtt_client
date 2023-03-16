/*
 * File: ConnectAcknowledge.h
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

#ifndef CONNECTACKNOWLEDGE
#define CONNECTACKNOWLEDGE

#include <stdint.h>
#include "Packet.h"
#include "PicoMqttProperties.h"
#include "types/VariableByteInteger.h"

namespace PicoMqtt
{
    typedef union
    {
        struct
        {
            uint16_t reserved : 7;
            uint16_t session : 1;
            uint16_t reasonCode : 8;
        };
        uint16_t data;
    } ConnackFlags;

    /**
     * @brief Represents a MQTT 5 Connect Acknowledge Packet
     *
     */
    class ConnectAcknowledge : public Packet
    {
    private:
        uint8_t state = 0;
        ConnackFlags flags;
        Properties properties;

    protected:
    public:
        ConnectAcknowledge();
        size_t size() { return 0; };
        size_t pushToClient(Client *) { return 0; };
        bool readFromClient(Client *, uint32_t *);
    };
}

#endif /* CONNECTACKNOWLEDGE */
