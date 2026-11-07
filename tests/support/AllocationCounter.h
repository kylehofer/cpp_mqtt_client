#ifndef TESTS_SUPPORT_ALLOCATIONCOUNTER
#define TESTS_SUPPORT_ALLOCATIONCOUNTER

#include <stddef.h>

/**
 * @brief Counts heap allocations (malloc, calloc, realloc, aligned allocation and
 * so operator new) made between start() and stop(), by replacing the allocator
 * for the test binary. Only available with glibc and without sanitizers, which
 * replace the allocator themselves.
 */
namespace AllocationCounter
{
    bool available();
    void start();
    /**
     * @return size_t Allocations since start()
     */
    size_t stop();
}

#endif /* TESTS_SUPPORT_ALLOCATIONCOUNTER */
