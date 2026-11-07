#ifndef PACKETBUFFER
#define PACKETBUFFER

#include <stdio.h>
#include <stdint.h>
#include "CppMqttConfig.h"
#include "SocketClient.h"
#include "TransmitBuffer.h"

/**
 * @brief Destination for an encoded packet. Packets push their bytes into it in
 * order. There are four kinds:
 *  - Transmit buffer, PacketBuffer(transmitBuffer, length): bytes are written
 *    straight into memory owned by the network client and committed in one
 *    piece by flush(), so the packet is copied once and sent whole or not at all.
 *  - Streaming, PacketBuffer(client, length): bytes are written to the client in
 *    chunks of CPP_MQTT_WRITE_CHUNK_SIZE as they are pushed. Nothing is
 *    allocated and the whole packet is never held in memory. This is how the
 *    client sends packets.
 *  - Caller memory, PacketBuffer(storage, capacity): bytes are copied into
 *    storage the caller provides.
 *  - Heap, PacketBuffer(size): dynamic memory only; allocates size bytes.
 *
 * Pushing more than fits, or more than the declared length when streaming, is
 * refused and marks the buffer as overflowed.
 */
class PacketBuffer
{
private:
    // Memory target (caller memory or heap); NULL when streaming
    uint8_t *buffer = NULL;
    size_t capacity = 0;
    size_t used = 0;
    bool ownsBuffer = false;

    // Transmit buffer target
    TransmitBuffer *transmitBuffer = NULL;
    bool committed = false;

    // Streaming target
    Client *client = NULL;
    size_t length = 0;
    size_t pushed = 0;
    uint8_t chunk[CPP_MQTT_WRITE_CHUNK_SIZE];
    size_t chunkUsed = 0;
    bool writeFailed = false;

    bool overflow = false;

protected:
public:
    /**
     * @brief Streams a packet of exactly length bytes to client
     */
    PacketBuffer(Client *client, size_t length);

    /**
     * @brief Writes a packet of exactly length bytes into a network client's
     * transmit buffer, which must have at least length bytes writable
     */
    PacketBuffer(TransmitBuffer *transmitBuffer, size_t length);

    /**
     * @brief Writes into caller-provided storage of capacity bytes
     */
    PacketBuffer(uint8_t *storage, size_t capacity);

#ifndef STATIC_MEMORY
    PacketBuffer() : PacketBuffer((size_t)0){};
    /**
     * @brief Allocates size bytes on the heap. Not available with STATIC_MEMORY.
     */
    PacketBuffer(size_t size);
#endif

    ~PacketBuffer();
    PacketBuffer(const PacketBuffer &) = delete;
    PacketBuffer &operator=(const PacketBuffer &) = delete;

    /**
     * @brief Appends data. Writes nothing and marks the buffer as overflowed if
     * the data does not fit.
     *
     * @return size_t The number of bytes accepted
     */
    size_t push(const void *input, size_t size);
    size_t push(uint8_t value)
    {
        return push(&value, 1);
    };

    /**
     * @brief When streaming, writes any buffered bytes to the client. With a
     * transmit buffer, commits the packet if exactly its declared length was
     * pushed; otherwise nothing is committed and nothing is sent.
     *
     * @return true If everything pushed so far has been written (or committed),
     * the declared length was not exceeded and no write failed
     */
    bool flush();

    /**
     * @brief Whether more was pushed than fits (or than the declared length when
     * streaming), or the heap allocation failed. An overflowed memory buffer
     * must not be sent.
     */
    bool overflowed() { return overflow; };

    /**
     * @brief Whether a write to the client accepted fewer bytes than requested
     */
    bool failed() { return writeFailed; };

    /**
     * @return uint8_t* The bytes written so far; NULL when streaming
     */
    uint8_t *getBuffer() { return buffer; };

    /**
     * @return size_t Bytes written so far to memory, or pushed when streaming
     */
    size_t getLength() { return (client != NULL || transmitBuffer != NULL) ? pushed : used; };
};

#endif /* PACKETBUFFER */
