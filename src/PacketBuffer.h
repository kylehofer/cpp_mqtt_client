#ifndef PACKETBUFFER
#define PACKETBUFFER

#include <stdio.h>
#include <stdint.h>
class PacketBuffer
{
private:
#ifdef STATIC_MEMORY
#define MAX_PACKET_BUFFER_SIZE 4096
    uint8_t buffer[MAX_PACKET_BUFFER_SIZE];
#else
    uint8_t *buffer = NULL;
#endif
    uint8_t *position;
    size_t length;

protected:
public:
    PacketBuffer() : PacketBuffer(0){};
    PacketBuffer(size_t size);
    ~PacketBuffer();
    size_t push(const void *input, size_t size);
    size_t push(uint8_t value)
    {
        return push(&value, 1);
    };

    uint8_t *getBuffer() { return buffer; };
    size_t getLength() { return length; };
};

#endif /* PACKETBUFFER */
