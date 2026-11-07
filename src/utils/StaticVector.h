/*
 * File: StaticVector.h
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

#ifndef SRC_UTILS_STATICVECTOR
#define SRC_UTILS_STATICVECTOR

#include <stddef.h>
#include "CppMqttConfig.h"

#ifndef STATIC_MEMORY
#include <vector>
#endif

namespace CppMqtt
{
    /**
     * @brief A vector with a fixed capacity and no dynamic allocation.
     * Supports the subset of std::vector used by this library. Adding to a full
     * vector is refused: push_back returns false and the vector is unchanged.
     *
     * @tparam T Element type; must be default constructible and copyable
     * @tparam N Capacity
     */
    template <typename T, size_t N>
    class StaticVector
    {
        static_assert(N > 0, "StaticVector capacity must be at least 1");

    private:
        T items[N];
        size_t count = 0;

    public:
        typedef T *iterator;
        typedef const T *const_iterator;

        bool push_back(const T &value)
        {
            if (count >= N)
            {
                return false;
            }
            items[count++] = value;
            return true;
        }

        void pop_back()
        {
            if (count > 0)
            {
                count--;
            }
        }

        /**
         * @brief Removes the element at position, keeping the order of the rest
         *
         * @return iterator The element after the removed one
         */
        iterator erase(iterator position)
        {
            if (position < begin() || position >= end())
            {
                return end();
            }
            for (iterator next = position + 1; next < end(); next++)
            {
                *(next - 1) = *next;
            }
            count--;
            return position;
        }

        void clear() { count = 0; }

        T &back() { return items[count - 1]; }
        const T &back() const { return items[count - 1]; }
        T &operator[](size_t index) { return items[index]; }
        const T &operator[](size_t index) const { return items[index]; }

        iterator begin() { return items; }
        iterator end() { return items + count; }
        const_iterator begin() const { return items; }
        const_iterator end() const { return items + count; }

        size_t size() const { return count; }
        bool empty() const { return count == 0; }
        bool full() const { return count >= N; }
        static constexpr size_t capacity() { return N; }
    };

    /**
     * @brief The container type used by the library: a StaticVector of capacity N
     * with STATIC_MEMORY, otherwise a std::vector (N is ignored).
     */
#ifdef STATIC_MEMORY
    template <typename T, size_t N>
    using Vector = StaticVector<T, N>;
#else
    template <typename T, size_t N>
    using Vector = std::vector<T>;
#endif

    /**
     * @brief Appends to a Vector in either memory mode
     *
     * @return true If the value was added
     * @return false If a static vector was full
     */
    template <typename T, size_t N>
    inline bool pushBack(StaticVector<T, N> &vector, const T &value)
    {
        return vector.push_back(value);
    }

#ifndef STATIC_MEMORY
    template <typename T>
    inline bool pushBack(std::vector<T> &vector, const T &value)
    {
        vector.push_back(value);
        return true;
    }
#endif

    /**
     * @brief Whether a Vector can take no more elements. Always false for a std::vector.
     */
    template <typename T, size_t N>
    inline bool isFull(const StaticVector<T, N> &vector)
    {
        return vector.full();
    }

#ifndef STATIC_MEMORY
    template <typename T>
    inline bool isFull(const std::vector<T> &)
    {
        return false;
    }
#endif
}

#endif /* SRC_UTILS_STATICVECTOR */
