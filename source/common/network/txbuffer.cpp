#include <network/txbuffer.h>

#include <endian.h>

TxBuffer &operator<<(TxBuffer &tx, uint8_t value) {
    tx.pushNetworkOrder(&value, sizeof value);
    return tx;
}
TxBuffer &operator<<(TxBuffer &tx, int8_t value) {
    return (tx << static_cast<uint8_t>(value));
}
TxBuffer &operator<<(TxBuffer &tx, uint16_t value) {
    value = htobe16(value);
    tx.pushNetworkOrder(&value, sizeof value);
    return tx;
}
TxBuffer &operator<<(TxBuffer &tx, int16_t value) {
    return (tx << static_cast<uint16_t>(value));
}
TxBuffer &operator<<(TxBuffer &tx, uint32_t value) {
    value = htobe32(value);
    tx.pushNetworkOrder(&value, sizeof value);
    return tx;
}
TxBuffer &operator<<(TxBuffer &tx, int32_t value) {
    return (tx << static_cast<uint32_t>(value));
}
TxBuffer &operator<<(TxBuffer &tx, uint64_t value) {
    value = htobe64(value);
    tx.pushNetworkOrder(&value, sizeof value);
    return tx;
}
TxBuffer &operator<<(TxBuffer &tx, int64_t value) {
    return (tx << static_cast<uint64_t>(value));
}
TxBuffer &operator<<(TxBuffer &tx, float value) {
    uint32_t bits = *static_cast<uint32_t *>(static_cast<void *>(&value));
    return (tx << bits);
}
TxBuffer &operator<<(TxBuffer &tx, double value) {
    uint64_t bits = *static_cast<uint64_t *>(static_cast<void *>(&value));
    return (tx << bits);
}

TxBuffer &operator<<(TxBuffer &tx, const std::string &value) {
    tx << static_cast<uint32_t>(value.size());
    for(char character : value)
        tx << static_cast<uint8_t>(character);
    return tx;
}