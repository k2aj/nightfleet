#include <network/nbuffer.h>

#include <stdexcept>
#include <cassert>
#include <cstring>

void NetworkBuffer::pushNetworkOrder(const void *ptr, size_t numBytes) {
    auto bytePtr = static_cast<const uint8_t *>(ptr);
    store.insert(store.end(), bytePtr, bytePtr+numBytes);
}
const uint8_t *NetworkBuffer::ptr() const {
    return &store[0] + position;
}
size_t NetworkBuffer::size() const {
    return store.size() - position;
}
void NetworkBuffer::pop(size_t numBytes) {
    if(position + numBytes > store.size())
        throw std::out_of_range("Attempting to pop more bytes from buffer than available.");
    position += numBytes;
}
void NetworkBuffer::maybeCompact() {
    if(store.size() >= 2*size())
        compact();
}
void NetworkBuffer::compact() {
    store.erase(store.begin(), store.begin()+position);
    store.shrink_to_fit();
    position = 0;
}

size_t NetworkBuffer::getPosition() const {
    return position;
}
void NetworkBuffer::setPosition(size_t newPosition) {
    if(newPosition >= store.size())
        throw std::out_of_range("Attempting to setPosition beyond buffer bounds.");
    position = newPosition;
}