/*
 * File: Endian.h
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

#ifndef SRC_TYPES_ENDIAN
#define SRC_TYPES_ENDIAN

/**
 * Defines CPP_MQTT_BIG_ENDIAN as 1 on big-endian hosts and 0 on little-endian hosts.
 *
 * FORCE_BIG_ENDIAN / FORCE_LITTLE_ENDIAN override detection. Otherwise the
 * compiler's __BYTE_ORDER__ is used (GCC and Clang, including arm-none-eabi),
 * falling back to BYTE_ORDER from the C library. If neither is available the
 * build fails rather than silently guessing: an undefined BYTE_ORDER and
 * BIG_ENDIAN both evaluate to 0 in the preprocessor, which would wrongly
 * select big-endian.
 */
#if defined(FORCE_BIG_ENDIAN) && defined(FORCE_LITTLE_ENDIAN)
#error "Define only one of FORCE_BIG_ENDIAN and FORCE_LITTLE_ENDIAN"
#elif defined(FORCE_BIG_ENDIAN)
#define CPP_MQTT_BIG_ENDIAN 1
#elif defined(FORCE_LITTLE_ENDIAN)
#define CPP_MQTT_BIG_ENDIAN 0
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
#define CPP_MQTT_BIG_ENDIAN (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#elif defined(BYTE_ORDER) && defined(BIG_ENDIAN)
#define CPP_MQTT_BIG_ENDIAN (BYTE_ORDER == BIG_ENDIAN)
#else
#error "Unable to detect byte order; define FORCE_BIG_ENDIAN or FORCE_LITTLE_ENDIAN"
#endif

#endif /* SRC_TYPES_ENDIAN */
