/*
 * File: StringPairProperty.h
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

#ifndef STRINGPAIRPROPERTY
#define STRINGPAIRPROPERTY

#include <stdint.h>
#include <stdio.h>
#include "properties/Property.h"

namespace PicoMqtt
{
    class StringPairProperty : public Property
    {
    private:
        int8_t state;
        EncodedString key;
        EncodedString value;

    protected:
        size_t pushDataToBuffer(void *buffer);
        size_t pushPropertyToClient(Client *client);

    public:
        StringPairProperty();
        StringPairProperty(PropertyCodes identifier);
        StringPairProperty(PropertyCodes identifier, EncodedString key, EncodedString value);
        StringPairProperty(PropertyCodes identifier, const char *, uint32_t, const char *, uint32_t);
        size_t propertySize();
        bool readFromClient(Client *, uint32_t *);
        void setKey(EncodedString value);
        EncodedString getKey();
        void setValue(EncodedString value);
        EncodedString getValue();
    };
}
#endif /* STRINGPAIRPROPERTY */
