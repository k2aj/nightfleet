#include <network/protocol.h>

#include <util/version.h>
#include <util/time.h>

ProtocolError::ProtocolError(const std::string &what) : std::runtime_error(what) {}

RxBuffer &operator>>(RxBuffer &rx, MessageType &type) {
    uint32_t value;
    rx >> value;
    if(value > 0 && value < static_cast<uint32_t>(MessageType::COUNT))
        type = static_cast<MessageType>(value);
    else
        type = MessageType::UNKNOWN;
    return rx;
}
TxBuffer &operator<<(TxBuffer &tx, const MessageType &type) {
    return (tx << static_cast<uint32_t>(type));
}

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