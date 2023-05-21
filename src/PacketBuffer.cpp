#include "PacketBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PacketBuffer::PacketBuffer(size_t size)
{
    buffer = position = (uint8_t *)malloc(size);
}

PacketBuffer::~PacketBuffer()
{
    if (buffer != NULL)
    {
        free(buffer);
    }
}

inline size_t PacketBuffer::push(const void *input, size_t size)
{
    memcpy(position, input, size);
    position += size;
    return size;
}
