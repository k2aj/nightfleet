#pragma once

#include <stdexcept>

#include <network/message.h>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>
#include <util/time.h>

class ProtocolError : public std::runtime_error {
    public:
    ProtocolError(const std::string &what);
};

enum class MessageType : uint32_t {
    UNKNOWN = 0,
    VERSION = 1,
    COUNT = 2
};

RxBuffer &operator>>(RxBuffer &rx, MessageType &type);
TxBuffer &operator<<(TxBuffer &tx, const MessageType &type);

/** Checks if client and server application versions match.
 *  @returns true if client and server application versions are compatible with each other.
 *  @throw TimeoutError
 *  @throw ProtocolError 
 */
[[nodiscard]]
bool performVersionHandshake(MessageSocket &, const Duration &timeout);
