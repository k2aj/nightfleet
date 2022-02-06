#pragma once

#include <string>
#include <stdexcept>

#include <network/message.h>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>
#include <util/time.h>

/** Checks if client and server application versions match.
 *  @returns true if client and server application versions are compatible with each other.
 *  @throw TimeoutError
 *  @throw ProtocolError 
 */
[[nodiscard]]
bool performVersionHandshake(MessageSocket &, const Duration &timeout);

/** Thrown when the other side of communication does not obey 
 *  the application layer communication protocol. 
 */
class ProtocolError : public std::runtime_error {
    public:
    ProtocolError(const std::string &what);
};

enum class MessageType : uint32_t {
    UNKNOWN = 0,
    VERSION = 1,
    LOGIN_REQUEST = 2,
    LOGIN_RESPONSE = 3,
    COUNT = 4
};

struct LoginRequest {
    std::string username;
};

enum class LoginResponse : uint32_t {
    OK = 0,
    E_ALREADY_LOGGED_IN = 1,
    COUNT = 2
};



#define DECLARE_SERDE(Type) \
    RxBuffer &operator>>(RxBuffer &rx, Type &value); \
    TxBuffer &operator<<(TxBuffer &tx, const Type &value);

DECLARE_SERDE(MessageType)
DECLARE_SERDE(LoginRequest)
DECLARE_SERDE(LoginResponse)

#undef DECLARE_ENUM_SERDE