#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/** Buffer for data sent/received over the network.
 *  Stores data in network byte order (big endian).
 *  Has FIFO queue semantics (data is inserted at the back and extracted from the front),
 *  but is actually backed by a dynamic array (works with functions like send, recv etc.)
 */
class NetworkBuffer {
    public:

    /** Appends raw data at the end of the buffer.
     *  The appended data is assumed to use network byte order; 
     *  this function performs no byte order conversion.
     */
    void pushNetworkOrder(const void *ptr, size_t numBytes);

    /// @returns Pointer to the first byte in the buffer.
    const uint8_t *ptr() const;

    /// @returns Number of bytes stored in the buffer.
    size_t size() const;

    /// Removes the specified number of bytes from the front of the buffer.
    void pop(size_t numBytes);

    /// equivalent to std::vector::shrink_to_fit
    void compact();

    void maybeCompact();

    size_t getPosition() const;
    void setPosition(size_t newPosition);

    private:
    std::vector<uint8_t> store;
    size_t position = 0;
};