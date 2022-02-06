#pragma once

#include <string>
#include <stdexcept>
#include <set>

#include <network/message.h>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>
#include <util/time.h>
#include <util/version.h>

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
    ECHO = 4,
    ALERT = 5,
    COUNT = 6
};

struct LoginRequest {
    std::string username;
};

enum class LoginResponse : uint32_t {
    OK = 0,
    E_ALREADY_LOGGED_IN = 1,
    COUNT = 2
};

struct EchoRequest {
    std::string message;
};

struct AlertRequest {
    std::string message;
};

#define DECLARE_SERDE(Type) \
    RxBuffer &operator>>(RxBuffer &rx, Type &value); \
    TxBuffer &operator<<(TxBuffer &tx, const Type &value);

DECLARE_SERDE(MessageType)
DECLARE_SERDE(LoginRequest)
DECLARE_SERDE(LoginResponse)
DECLARE_SERDE(EchoRequest)
DECLARE_SERDE(AlertRequest)

#undef DECLARE_ENUM_SERDE

class NFProtocolEntity {
    public:

    NFProtocolEntity(int sockfd);

    void runNetworkEvents();
    void halt();
    bool isRunning() const;
    void setTimeout(const Duration &timeoutDuration = 5s);

    void sendVersionHandshake(const Version &);
    void sendLoginRequest(const LoginRequest &);
    void sendLoginResponse(LoginResponse);
    void sendEchoRequest(const EchoRequest &);
    void sendAlertRequest(const AlertRequest &);
    
    virtual void onInit();
    virtual void onUpdate(const Duration &dt);
    virtual void onVersionHandshake(const Version &);
    virtual void onLoginRequest(const LoginRequest &);
    virtual void onLoginResponse(LoginResponse);
    virtual void onEchoRequest(const EchoRequest &);
    virtual void onAlertRequest(const AlertRequest &);

    virtual void onProtocolError(const ProtocolError &e);
    virtual void onTimeout();
    virtual void onDisconnect();

    protected:
    std::set<MessageType> whitelist, blacklist;

    private:
    MessageSocket msock;
    bool _running = true;
    bool _timeoutActive = false;
    TimePoint timeoutDeadline;
};