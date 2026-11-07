/*
 * File: PropertyPool.h
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

#ifndef SRC_PROPERTIES_PROPERTYPOOL
#define SRC_PROPERTIES_PROPERTYPOOL

#include <stddef.h>
#include <new>
#include <utility>
#include "CppMqttConfig.h"
#include "properties/Property.h"
#include "properties/ByteProperty.h"
#include "properties/WordProperty.h"
#include "properties/DWordProperty.h"
#include "properties/VariableByteProperty.h"
#include "properties/StringProperty.h"
#include "properties/StringPairProperty.h"

namespace CppMqtt
{
#ifdef STATIC_MEMORY
    namespace detail
    {
        /**
         * @brief Storage for one property of any type. Every property class derives
         * from one of these and adds no members.
         */
        union AnyProperty
        {
            ByteProperty byteProperty;
            WordProperty wordProperty;
            DWordProperty dWordProperty;
            VariableByteProperty variableByteProperty;
            StringProperty stringProperty;
            StringPairProperty stringPairProperty;
            AnyProperty() {}
            ~AnyProperty() {}
        };

        /**
         * @return void* A free slot of sizeof(AnyProperty) bytes, or NULL if all
         * CPP_MQTT_PROPERTY_POOL_SIZE slots are in use
         */
        void *allocatePropertySlot();
        void releasePropertySlot(void *slot);
    }
#endif

    /**
     * @brief Creates a property. With STATIC_MEMORY it is placed in a fixed pool of
     * CPP_MQTT_PROPERTY_POOL_SIZE slots shared by the whole program; otherwise it
     * is allocated with new. Destroy it with destroyProperty().
     *
     * @return T* The property, or NULL if the static pool is exhausted
     */
    template <typename T, typename... Args>
    T *createProperty(Args &&...args)
    {
#ifdef STATIC_MEMORY
        static_assert(sizeof(T) <= sizeof(detail::AnyProperty), "Property type is larger than a pool slot");
        static_assert(alignof(T) <= alignof(detail::AnyProperty), "Property type needs more alignment than a pool slot");
        void *slot = detail::allocatePropertySlot();
        if (slot == NULL)
        {
            return NULL;
        }
        return new (slot) T(std::forward<Args>(args)...);
#else
        return new T(std::forward<Args>(args)...);
#endif
    }

    /**
     * @brief Destroys a property made by createProperty()
     *
     * @param property The property, may be NULL
     */
    void destroyProperty(Property *property);

    /**
     * @brief The number of properties that can still be created. With dynamic
     * memory there is no fixed limit and this returns SIZE_MAX.
     */
    size_t propertyPoolAvailable();
}

#endif /* SRC_PROPERTIES_PROPERTYPOOL */
