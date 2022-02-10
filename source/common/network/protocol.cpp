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

RxBuffer &operator>>(RxBuffer &rx, AlertRequest &request) {
    return (rx >> request.message);
}
TxBuffer &operator<<(TxBuffer &tx, const AlertRequest &request) {
    return (tx << request.message);
}

RxBuffer &operator>>(RxBuffer &rx, EchoRequest &request) {
    return (rx >> request.message);
}
TxBuffer &operator<<(TxBuffer &tx, const EchoRequest &request) {
    return (tx << request.message);
}

RxBuffer &operator>>(RxBuffer &rx, JoinGameRequest &request) {
    return (rx >> request.gameID);
}
TxBuffer &operator<<(TxBuffer &tx, const JoinGameRequest &request) {
    return (tx << request.gameID);
}

// TODO fill these stubs
RxBuffer &operator>>(RxBuffer &rx, GameFullSync &s) {
    return rx;
}
TxBuffer &operator<<(TxBuffer &tx, const GameFullSync &s) {
    return tx;
}

// TODO fill these stubs
RxBuffer &operator>>(RxBuffer &rx, GameIncrementalSync &s) {
    return rx;
}
TxBuffer &operator<<(TxBuffer &tx, const GameIncrementalSync &s) {
    return tx;
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

NFProtocolEntity::NFProtocolEntity(int sockfd) :
    msock(sockfd)
{}

void NFProtocolEntity::runNetworkEvents() {
    msock.update();

    if(!msock.isConnected())
        onDisconnect();

    if(!msock.hasMessage() && _timeoutActive && Clock::now() >= timeoutDeadline)
        onTimeout();

    while(msock.hasMessage() && isRunning()) {
        RxBuffer message = msock.receiveMessage();
        try {
            MessageType type = message.read<MessageType>();
            
            if(!blacklist.empty() && blacklist.find(type) != blacklist.end())
                throw ProtocolError("Blacklisted message type received.");

            if(!whitelist.empty() && whitelist.find(type) == whitelist.end())
                throw ProtocolError("Message type not whitelisted.");

            switch(type) {

                #define DISPATCH(typetag, type, method) \
                case MessageType::typetag: { \
                    auto value = message.read<type>(); \
                    if(message.size() > 0) \
                        throw ProtocolError("Message too long!"); \
                    method(value); \
                    break; \
                }

                DISPATCH(VERSION,        Version,       onVersionHandshake)
                DISPATCH(LOGIN_REQUEST,  LoginRequest,  onLoginRequest)
                DISPATCH(LOGIN_RESPONSE, LoginResponse, onLoginResponse)
                DISPATCH(ECHO,           EchoRequest,   onEchoRequest)
                DISPATCH(ALERT,          AlertRequest,  onAlertRequest)
                DISPATCH(GAME_FULL_SYNC, GameFullSync,  onFullSync)
                DISPATCH(GAME_INCREMENTAL_SYNC, GameIncrementalSync, onIncrementalSync)

                #undef DISPATCH

                default:
                    throw ProtocolError("Unknown message type.");
            }

            _timeoutActive = false;

        } catch (const ProtocolError &e) {
            onProtocolError(e);
        } catch (const std::out_of_range &) {
            onProtocolError(ProtocolError("Message too short."));
        }
    }
}

void NFProtocolEntity::halt() {
    _running = false;
}

bool NFProtocolEntity::isRunning() const {
    return _running;
}

void NFProtocolEntity::setTimeout(const Duration &d) {
    _timeoutActive = true;
    timeoutDeadline = Clock::now() + d;
}

void NFProtocolEntity::sendVersionHandshake(const Version &v) {
    TxBuffer message;
    message << MessageType::VERSION << v;
    msock.sendMessage(message);
}
void NFProtocolEntity::sendLoginRequest(const LoginRequest &r) {
    TxBuffer message;
    message << MessageType::LOGIN_REQUEST << r;
    msock.sendMessage(message);
}
void NFProtocolEntity::sendLoginResponse(LoginResponse r) {
    TxBuffer message;
    message << MessageType::LOGIN_RESPONSE << r;
    msock.sendMessage(message);
}
void NFProtocolEntity::sendEchoRequest(const EchoRequest &r) {
    TxBuffer message;
    message << MessageType::ECHO << r;
    msock.sendMessage(message);
}
void NFProtocolEntity::sendAlertRequest(const AlertRequest &r) {
    TxBuffer message;
    message << MessageType::ALERT << r;
    msock.sendMessage(message);
}
void NFProtocolEntity::sendJoinGameRequest(const JoinGameRequest &r) {
    TxBuffer message;
    message << MessageType::JOIN_GAME << r;
    msock.sendMessage(message);
}
void NFProtocolEntity::sendFullSync(const GameFullSync &s) {
    TxBuffer message;
    message << MessageType::GAME_FULL_SYNC << s;
    msock.sendMessage(message);
}
void NFProtocolEntity::sendIncrementalSync(const GameIncrementalSync &s) {
    TxBuffer message;
    message << MessageType::GAME_INCREMENTAL_SYNC << s;
    msock.sendMessage(message);
}
void NFProtocolEntity::onInit() {}
void NFProtocolEntity::onUpdate(const Duration &dt) {}
void NFProtocolEntity::onVersionHandshake(const Version &) {}
void NFProtocolEntity::onLoginRequest(const LoginRequest &) {throw ProtocolError("Unexpected LoginRequest.");}
void NFProtocolEntity::onLoginResponse(LoginResponse) {throw ProtocolError("Unexpected LoginResponse.");}
void NFProtocolEntity::onEchoRequest(const EchoRequest &) {throw ProtocolError("Unexpected EchoRequest.");}
void NFProtocolEntity::onAlertRequest(const AlertRequest &) {throw ProtocolError("Unexpected EchoResponse.");}
void NFProtocolEntity::onJoinGameRequest(const JoinGameRequest &) {throw ProtocolError("Unexpected JoinGameRequest.");}
void NFProtocolEntity::onFullSync(const GameFullSync &) {throw ProtocolError("Unexpected FullSync.");}
void NFProtocolEntity::onIncrementalSync(const GameIncrementalSync &) {throw ProtocolError("Unexpected IncrementalSync.");}

void NFProtocolEntity::onTimeout() {onDisconnect();}