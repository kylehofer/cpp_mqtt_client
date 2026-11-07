#ifndef TESTS_UTILS_TESTPACKETBUFFER
#define TESTS_UTILS_TESTPACKETBUFFER

#include <stdint.h>
#include <stddef.h>
#include "PacketBuffer.h"

/**
 * @brief Storage for TestPacketBuffer, a base so it is constructed before the
 * PacketBuffer that points into it.
 */
template <size_t N>
struct TestPacketBufferStorage
{
    uint8_t storage[N];
};

/**
 * @brief A PacketBuffer that owns its memory, for inspecting encoded bytes in
 * tests. Allocation free, so it works with and without STATIC_MEMORY.
 */
template <size_t N = 4096>
class SizedTestPacketBuffer : private TestPacketBufferStorage<N>, public PacketBuffer
{
public:
    SizedTestPacketBuffer() : TestPacketBufferStorage<N>(), PacketBuffer(this->storage, N) {}
};

typedef SizedTestPacketBuffer<> TestPacketBuffer;

#endif /* TESTS_UTILS_TESTPACKETBUFFER */
