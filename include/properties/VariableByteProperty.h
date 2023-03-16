/*
 * File: VariableByteProperty.h
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

#ifndef VARIABLEBYTEPROPERTY
#define VARIABLEBYTEPROPERTY

#include <stdint.h>
#include <stdio.h>
#include "properties/Property.h"

namespace PicoMqtt
{
    /**
     * @brief Represents a MQTT 5 Variable Byte property
     * Used for reading and writing a Variable Bytes (1 to 4 bytes) to a communication client
     */
    class VariableByteProperty : public Property
    {
    private:
        VariableByteInteger value;

    protected:
        size_t pushDataToBuffer(void *buffer);
        size_t pushPropertyToClient(Client *client);

    public:
        VariableByteProperty();
        VariableByteProperty(PropertyCodes identifier);
        VariableByteProperty(PropertyCodes identifier, VariableByteInteger value);
        size_t propertySize();
        bool readFromClient(Client *, uint32_t *);
        void setValue(VariableByteInteger value);
        void setValue(uint32_t value);
        VariableByteInteger getValue();
    };

}
#endif /* VARIABLEBYTEPROPERTY */
