#pragma once

#include <network/rxbuffer.h>
#include <network/txbuffer.h>

struct Version {
    int32_t major, minor, patch;
};

TxBuffer &operator<<(TxBuffer &, const Version &);
RxBuffer &operator>>(RxBuffer &, Version &);

/// Used to check if client & server are compatible (currently not implemented)
constexpr Version applicationVersion{0,1,0};