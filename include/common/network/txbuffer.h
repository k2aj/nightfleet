#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <set>
#include <map>
#include <string>

#include <network/nbuffer.h>

/** Buffer for data sent over the network.
 *  
 *  Stores data internally in network byte order. Automatically converts
 *  inserted values to network byte order.
 */
class TxBuffer : public NetworkBuffer {};

TxBuffer &operator<<(TxBuffer &tx, uint8_t value);
TxBuffer &operator<<(TxBuffer &tx, int8_t value);
TxBuffer &operator<<(TxBuffer &tx, uint16_t value);
TxBuffer &operator<<(TxBuffer &tx, int16_t value);
TxBuffer &operator<<(TxBuffer &tx, uint32_t value);
TxBuffer &operator<<(TxBuffer &tx, int32_t value);
TxBuffer &operator<<(TxBuffer &tx, uint64_t value);
TxBuffer &operator<<(TxBuffer &tx, int64_t value);
TxBuffer &operator<<(TxBuffer &tx, float value);
TxBuffer &operator<<(TxBuffer &tx, double value);

TxBuffer &operator<<(TxBuffer &tx, const std::string &value);

template<typename T>
TxBuffer &operator<<(TxBuffer &tx, const std::vector<T> &value) {
    tx << static_cast<uint32_t>(value.size());
    for(const auto &element : value)
        tx << element;
    return tx;
}

template<typename T>
TxBuffer &operator<<(TxBuffer &tx, const std::set<T> &value) {
    tx << static_cast<uint32_t>(value.size());
    for(const auto &element : value)
        tx << element;
    return tx;
}

template<typename K, typename V>
TxBuffer &operator<<(TxBuffer &tx, const std::map<K,V> &value) {
    tx << static_cast<uint32_t>(value.size());
    for(const auto &entry : value)
        tx << entry.first << entry.second;
    return tx;
}