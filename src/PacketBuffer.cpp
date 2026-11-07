#include "PacketBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PacketBuffer::PacketBuffer(Client *client, size_t length) : client(client), length(length)
{
}

PacketBuffer::PacketBuffer(TransmitBuffer *transmitBuffer, size_t length) : transmitBuffer(transmitBuffer), length(length)
{
    overflow = (transmitBuffer == NULL || transmitBuffer->writable() < length);
}

PacketBuffer::PacketBuffer(uint8_t *storage, size_t capacity) : buffer(storage), capacity(capacity)
{
    overflow = (storage == NULL && capacity > 0);
}

#ifndef STATIC_MEMORY
PacketBuffer::PacketBuffer(size_t size)
{
    buffer = (uint8_t *)malloc(size);
    ownsBuffer = true;
    capacity = (buffer != NULL) ? size : 0;
    overflow = (size > 0 && buffer == NULL);
}
#endif

PacketBuffer::~PacketBuffer()
{
#ifndef STATIC_MEMORY
    if (ownsBuffer && buffer != NULL)
    {
        free(buffer);
    }
#endif
}

size_t PacketBuffer::push(const void *input, size_t size)
{
    if (size == 0)
    {
        return 0;
    }

    if (transmitBuffer != NULL)
    {
        if (committed || overflow || size > length - pushed)
        {
            overflow = true;
            return 0;
        }

        // Encode straight into the network client's memory, across its wrap if needed
        const uint8_t *source = (const uint8_t *)input;
        size_t remaining = size;
        while (remaining > 0)
        {
            size_t contiguous = 0;
            uint8_t *destination = transmitBuffer->writePointer(pushed + (size - remaining), contiguous);
            if (destination == NULL || contiguous == 0)
            {
                overflow = true;
                return 0;
            }
            size_t count = remaining < contiguous ? remaining : contiguous;
            memcpy(destination, source, count);
            source += count;
            remaining -= count;
        }
        pushed += size;
        return size;
    }

    if (client != NULL)
    {
        // Never write past the declared length: the receiver would lose framing
        if (overflow || writeFailed || size > length - pushed)
        {
            overflow = overflow || size > length - pushed;
            return 0;
        }

        const uint8_t *source = (const uint8_t *)input;
        size_t remaining = size;
        while (remaining > 0)
        {
            size_t space = sizeof(chunk) - chunkUsed;
            size_t count = remaining < space ? remaining : space;
            memcpy(chunk + chunkUsed, source, count);
            chunkUsed += count;
            source += count;
            remaining -= count;
            if (chunkUsed == sizeof(chunk) && !flush())
            {
                return 0;
            }
        }
        pushed += size;
        return size;
    }

    if (overflow || size > capacity - used)
    {
        overflow = true;
        return 0;
    }
    memcpy(buffer + used, input, size);
    used += size;
    return size;
}

bool PacketBuffer::flush()
{
    if (transmitBuffer != NULL)
    {
        if (committed)
        {
            return true;
        }
        // Only a complete packet is ever committed, so a failure sends nothing
        if (overflow || writeFailed || pushed != length)
        {
            return false;
        }
        if (!transmitBuffer->commit(pushed))
        {
            writeFailed = true;
            return false;
        }
        // Committed once; a second flush must not commit again
        committed = true;
        return true;
    }

    if (client != NULL && chunkUsed > 0 && !writeFailed)
    {
        size_t written = client->write(chunk, chunkUsed);
        writeFailed = written < chunkUsed;
        chunkUsed = 0;
    }
    return !overflow && !writeFailed;
}
