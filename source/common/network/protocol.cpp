#include <network/protocol.h>

#include <util/version.h>
#include <util/time.h>

bool performVersionHandshake(MessageSocket &s, const Duration &timeout) {

    TxBuffer request;
    request << MessageType::VERSION << applicationVersion;
    s.sendMessage(request);
    s.waitForMessage(timeout);

    RxBuffer response = s.receiveMessage();

    try {
        // If we don't get a version message type, then we probably 
        // accidentally connected to a completely different application.
        MessageType responseType;
        response >> responseType;
        if(responseType != MessageType::VERSION)
            throw ProtocolError("Invalid message type.");
        
        Version remoteVersion;
        response >> remoteVersion;

        if(response.size() > 0)
            throw ProtocolError("Response too long.");

        return applicationVersion.isCompatibleWith(remoteVersion);
        
    } catch(const std::out_of_range &) {
        throw ProtocolError("Response too short.");
    }
}



RxBuffer &operator>>(RxBuffer &rx, LoginRequest &request) {
    return (rx >> request.username);
}
TxBuffer &operator<<(TxBuffer &tx, const LoginRequest &request) {
    return (tx << request.username);
}

ProtocolError::ProtocolError(const std::string &what) : std::runtime_error(what) {}

#define DEFINE_ENUM_SERDE(Enum) \
    RxBuffer &operator>>(RxBuffer &rx, Enum &enumValue) { \
        uint32_t rawValue; \
        rx >> rawValue; \
        if(rawValue >= 0 && rawValue < static_cast<uint32_t>(Enum::COUNT)) \
            enumValue = static_cast<Enum>(rawValue); \
        else \
            throw ProtocolError("Invalid enum value."); \
        return rx; \
    } \
    TxBuffer &operator<<(TxBuffer &tx, const Enum &enumValue) { \
        return (tx << static_cast<uint32_t>(enumValue)); \
    }

DEFINE_ENUM_SERDE(MessageType)
DEFINE_ENUM_SERDE(LoginResponse)