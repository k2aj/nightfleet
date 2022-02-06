#include <network/rxbuffer.h>

#include <cstring>
#include <endian.h>
#include <stdexcept>

// For unsigned types: memcpy and byteswap if necessary
// also check bounds on every read to prevent exploits

RxBuffer &operator>>(RxBuffer &rx, uint8_t &result) {
    if(rx.size() < sizeof(result))
        throw std::out_of_range("");
    memcpy(&result, rx.ptr(), sizeof(result));
    rx.pop(sizeof(result));
    return rx;
}
RxBuffer &operator>>(RxBuffer &rx, uint16_t &result) {
    if(rx.size() < sizeof(result))
        throw std::out_of_range("");
    memcpy(&result, rx.ptr(), sizeof(result));
    result = be16toh(result);
    rx.pop(sizeof(result));
    return rx;
}
RxBuffer &operator>>(RxBuffer &rx, uint32_t &result) {
    if(rx.size() < sizeof(result))
        throw std::out_of_range("");
    memcpy(&result, rx.ptr(), sizeof(result));
    result = be32toh(result);
    rx.pop(sizeof(result));
    return rx;
}
RxBuffer &operator>>(RxBuffer &rx, uint64_t &result) {
    if(rx.size() < sizeof(result))
        throw std::out_of_range("");
    memcpy(&result, rx.ptr(), sizeof(result));
    result = be64toh(result);
    rx.pop(sizeof(result));
    return rx;
}

// For signed & floating point types: reinterpret_cast to reuse implementation for unsigned types

RxBuffer &operator>>(RxBuffer &rx, int8_t &result) {
    return (rx >> reinterpret_cast<uint8_t&>(result));
}
RxBuffer &operator>>(RxBuffer &rx, int16_t &result) {
    return (rx >> reinterpret_cast<uint16_t&>(result));
}
RxBuffer &operator>>(RxBuffer &rx, int32_t &result) {
    return (rx >> reinterpret_cast<uint32_t&>(result));
}
RxBuffer &operator>>(RxBuffer &rx, int64_t &result) {
    return (rx >> reinterpret_cast<uint64_t&>(result));
}
RxBuffer &operator>>(RxBuffer &rx, float &result) {
    return (rx >> reinterpret_cast<uint32_t&>(result));
}
RxBuffer &operator>>(RxBuffer &rx, double &result) {
    return (rx >> reinterpret_cast<uint64_t&>(result));
}

// Strings are length-prefixed; chars stored as uint8_t (assume ASCII/UTF-8)
RxBuffer &operator>>(RxBuffer &rx, std::string &result) {
    auto stringLength = rx.read<uint32_t>();
    result.clear();
    result.reserve(stringLength);
    for(uint32_t i=0; i<stringLength; ++i)
        result += static_cast<char>(rx.read<uint8_t>());
    return rx;
}