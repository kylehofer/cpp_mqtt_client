/*
 * File: ReasonsAcknowledge.cpp
 * Project: cpp_mqtt_client
 * Created Date: Thursday March 16th 2023
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

#include "packets/ReasonsAcknowledge.h"

using namespace PicoMqtt;

#define REASON_CODE_SIZE 1

size_t ReasonsAcknowledge::pushToClient(Client *client)
{
    // Not Required
    return 0;
}

bool ReasonsAcknowledge::readFromClient(Client *client, uint32_t *bytes)
{
    if (!Acknowledge::readFromClient(client, bytes))
    {
        uint32_t start = getRemainingLength();

        while (dataRemaining())
        {
            uint32_t read = 0;
            uint8_t reasonCode = 0;

            client->read(&reasonCode, REASON_CODE_SIZE);

            reasonCodes.push_back(reasonCode);

            readBytes(read);
        }

        *bytes += getRemainingLength() - start;
    }

    return dataRemaining();
}