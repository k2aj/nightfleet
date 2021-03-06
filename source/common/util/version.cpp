#include <util/version.h>

TxBuffer &operator<<(TxBuffer &tx, const Version &version) {
    return (tx << version.major << version.minor << version.patch);
}
RxBuffer &operator>>(RxBuffer &rx, Version &version) {
    return (rx >> version.major >> version.minor >> version.patch);
}

bool Version::isCompatibleWith(const Version &other) const {
    return major == other.major && minor == other.minor;
}