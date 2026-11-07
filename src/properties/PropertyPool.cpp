/*
 * File: PropertyPool.cpp
 * Project: cpp_mqtt_client
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
 */

#include <stdint.h>
#include "properties/PropertyPool.h"

namespace CppMqtt
{
#ifdef STATIC_MEMORY
    namespace
    {
        // Not thread safe: properties are expected to be created and destroyed
        // from the one thread or loop that runs the client.
        alignas(detail::AnyProperty) uint8_t slots[CPP_MQTT_PROPERTY_POOL_SIZE][sizeof(detail::AnyProperty)];
        bool used[CPP_MQTT_PROPERTY_POOL_SIZE];
        size_t inUse = 0;

        int slotIndex(void *pointer)
        {
            uint8_t *address = (uint8_t *)pointer;
            uint8_t *first = &slots[0][0];
            if (address < first || address >= first + sizeof(slots))
            {
                return -1;
            }
            size_t offset = address - first;
            if (offset % sizeof(detail::AnyProperty) != 0)
            {
                return -1;
            }
            return (int)(offset / sizeof(detail::AnyProperty));
        }
    }

    namespace detail
    {
        void *allocatePropertySlot()
        {
            for (size_t i = 0; i < CPP_MQTT_PROPERTY_POOL_SIZE; i++)
            {
                if (!used[i])
                {
                    used[i] = true;
                    inUse++;
                    return slots[i];
                }
            }
            return NULL;
        }

        void releasePropertySlot(void *slot)
        {
            int index = slotIndex(slot);
            if (index >= 0 && used[index])
            {
                used[index] = false;
                inUse--;
            }
        }
    }

    void destroyProperty(Property *property)
    {
        if (property == NULL)
        {
            return;
        }
        // Only properties from the pool can be destroyed; anything else was not
        // made by createProperty() and is left alone rather than corrupting the pool
        if (slotIndex(property) < 0)
        {
            return;
        }
        property->~Property();
        detail::releasePropertySlot(property);
    }

    size_t propertyPoolAvailable()
    {
        return CPP_MQTT_PROPERTY_POOL_SIZE - inUse;
    }
#else
    void destroyProperty(Property *property)
    {
        delete property;
    }

    size_t propertyPoolAvailable()
    {
        return SIZE_MAX;
    }
#endif
}
