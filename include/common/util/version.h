#pragma once

#include <network/rxbuffer.h>
#include <network/txbuffer.h>

struct Version {
    int32_t major, minor, patch;

    bool isCompatibleWith(const Version &other) const;
};

TxBuffer &operator<<(TxBuffer &, const Version &);
RxBuffer &operator>>(RxBuffer &, Version &);

/// Used to check if client & server are compatible
constexpr Version applicationVersion{0,1,0};