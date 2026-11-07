#include <stdlib.h>
#include <errno.h>
#include "support/AllocationCounter.h"

#if defined(__has_feature)
#if __has_feature(address_sanitizer) || __has_feature(memory_sanitizer)
#define CPP_MQTT_SANITIZED
#endif
#endif
#if defined(__SANITIZE_ADDRESS__)
#define CPP_MQTT_SANITIZED
#endif

#if defined(__GLIBC__) && !defined(CPP_MQTT_SANITIZED)

static volatile bool counting = false;
static volatile size_t allocations = 0;

extern "C"
{
    void *__libc_malloc(size_t size);
    void *__libc_calloc(size_t count, size_t size);
    void *__libc_realloc(void *pointer, size_t size);
    void *__libc_memalign(size_t alignment, size_t size);
    void __libc_free(void *pointer);

    // Replacing these in the executable replaces them for the whole program,
    // including libstdc++'s operator new
    void *malloc(size_t size)
    {
        if (counting)
            allocations++;
        return __libc_malloc(size);
    }

    void *calloc(size_t count, size_t size)
    {
        if (counting)
            allocations++;
        return __libc_calloc(count, size);
    }

    void *realloc(void *pointer, size_t size)
    {
        if (counting)
            allocations++;
        return __libc_realloc(pointer, size);
    }

    void *memalign(size_t alignment, size_t size)
    {
        if (counting)
            allocations++;
        return __libc_memalign(alignment, size);
    }

    void *aligned_alloc(size_t alignment, size_t size)
    {
        if (counting)
            allocations++;
        return __libc_memalign(alignment, size);
    }

    int posix_memalign(void **result, size_t alignment, size_t size)
    {
        if (counting)
            allocations++;
        void *pointer = __libc_memalign(alignment, size);
        if (pointer == NULL)
        {
            return ENOMEM;
        }
        *result = pointer;
        return 0;
    }

    void free(void *pointer)
    {
        __libc_free(pointer);
    }
}

namespace AllocationCounter
{
    bool available() { return true; }

    void start()
    {
        allocations = 0;
        counting = true;
    }

    size_t stop()
    {
        counting = false;
        return allocations;
    }
}

#else

namespace AllocationCounter
{
    bool available() { return false; }
    void start() {}
    size_t stop() { return 0; }
}

#endif
