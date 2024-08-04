#include "PacketBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

PacketBuffer::PacketBuffer(size_t size) : length(size)
{
#ifdef STATIC_MEMORY
    if (size > MAX_PACKET_BUFFER_SIZE)
    {
        throw std::logic_error("Required Packet size greater than available.");
    }
    position = buffer;
#else
    buffer = position = (uint8_t *)malloc(size);
#endif
}

PacketBuffer::~PacketBuffer()
{
#ifndef STATIC_MEMORY
    if (buffer != NULL)
    {
        free(buffer);
    }
#endif
}

size_t PacketBuffer::push(const void *input, size_t size)
{
    memcpy(position, input, size);
    position += size;
    return size;
}
