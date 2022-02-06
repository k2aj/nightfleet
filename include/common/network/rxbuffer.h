#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

#include <network/nbuffer.h>

/** Buffer for data received from the network.
 *  
 *  Stores data internally in network byte order. Automatically converts
 *  to host byte order when typed data is read from the buffer.
 */
class RxBuffer : public NetworkBuffer {

    public:

    template<typename T> T read() {
        T result;
        operator>>(*this, result);
        return result;
    }

    template<typename T> 
    void peek(size_t addr, T &result) {
        auto oldPos = getPosition();
        setPosition(addr+oldPos);
        operator>>(*this, result);
        setPosition(oldPos);
    }

    template<typename T> 
    T peek(size_t addr=0) {
        T result;
        peek<T>(0,result);
        return result;
    }
};


RxBuffer &operator>>(RxBuffer &rx, uint8_t &result);
RxBuffer &operator>>(RxBuffer &rx, int8_t &result);
RxBuffer &operator>>(RxBuffer &rx, uint16_t &result);
RxBuffer &operator>>(RxBuffer &rx, int16_t &result);
RxBuffer &operator>>(RxBuffer &rx, uint32_t &result);
RxBuffer &operator>>(RxBuffer &rx, int32_t &result);
RxBuffer &operator>>(RxBuffer &rx, uint64_t &result);
RxBuffer &operator>>(RxBuffer &rx, int64_t &result);
RxBuffer &operator>>(RxBuffer &rx, float &result);
RxBuffer &operator>>(RxBuffer &rx, double &result);
RxBuffer &operator>>(RxBuffer &rx, std::string &result);

template<typename T>
RxBuffer &operator>>(RxBuffer &rx, std::vector<T> &result) {
    uint32_t vectorSize = rx.read<uint32_t>();
    result.clear();
    result.reserve(vectorSize);
    for(uint32_t i=0; i<vectorSize; ++i)
        result.push_back(rx.read<T>());
    return rx;
}