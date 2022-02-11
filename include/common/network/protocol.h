#pragma once

#include <string>
#include <stdexcept>
#include <set>

#include <network/exceptions.h>
#include <network/message.h>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>
#include <network/serde_macros.h>
#include <util/time.h>
#include <util/version.h>
#include <engine/game.h>
#include <engine/move.h>

/** Checks if client and server application versions match.
 *  @returns true if client and server application versions are compatible with each other.
 *  @throw TimeoutError
 *  @throw ProtocolError 
 */
[[nodiscard]]
bool performVersionHandshake(MessageSocket &, const Duration &timeout);

enum class MessageType : uint32_t {
    UNKNOWN = 0,
    VERSION = 1,
    LOGIN_REQUEST = 2,
    LOGIN_RESPONSE = 3,
    ECHO = 4,
    ALERT = 5,
    HOST_GAME = 6,
    JOIN_GAME = 7,
    LEAVE_GAME = 8,
    GAME_JOIN_ERROR = 9,
    GAME_FULL_SYNC = 10,
    GAME_INCREMENTAL_SYNC = 11,
    COUNT = 12
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

//typedef int64_t GameID;

struct HostGameRequest {
    const Map *map;
};

struct JoinGameRequest {
    static constexpr GameID 
        JOIN_ANY = 0;

    GameID gameID = JOIN_ANY;
};

struct LeaveGameRequest {};

enum class GameJoinError : uint32_t {
    NO_ERROR = 0,
    GAME_DOESNT_EXIST = 1,
    GAME_ALREADY_RUNNING = 2,
    SERVER_FULL = 3,
    SERVER_SHUTTING_DOWN = 4,
    OTHER = 5,
    COUNT = 6
};

struct GameIncrementalSync {
    // TODO add extra validation via SHA256 or something
    std::vector<Move> moveList;
};

DECLARE_SERDE(MessageType)
DECLARE_SERDE(LoginRequest)
DECLARE_SERDE(LoginResponse)
DECLARE_SERDE(EchoRequest)
DECLARE_SERDE(AlertRequest)
DECLARE_SERDE(HostGameRequest)
DECLARE_SERDE(JoinGameRequest)
DECLARE_SERDE(LeaveGameRequest)
DECLARE_SERDE(GameJoinError)
DECLARE_SERDE(GameIncrementalSync)

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
    void sendJoinGameRequest(const JoinGameRequest &);
    void sendHostGameRequest(const HostGameRequest &);
    void sendLeaveGameRequest(const LeaveGameRequest &);
    void sendGameJoinError(GameJoinError);
    void sendFullSync(const Game &);
    void sendIncrementalSync(const GameIncrementalSync &);
    
    virtual void onInit();
    virtual void onUpdate(const Duration &dt);
    virtual void onVersionHandshake(const Version &);
    virtual void onLoginRequest(const LoginRequest &);
    virtual void onLoginResponse(LoginResponse);
    virtual void onEchoRequest(const EchoRequest &);
    virtual void onAlertRequest(const AlertRequest &);
    virtual void onHostGameRequest(const HostGameRequest &);
    virtual void onJoinGameRequest(const JoinGameRequest &);
    virtual void onLeaveGameRequest(const LeaveGameRequest &);
    virtual void onGameJoinError(GameJoinError);
    virtual void onFullSync(const Game &);
    virtual void onIncrementalSync(const GameIncrementalSync &);

    virtual void onProtocolError(const ProtocolError &e) = 0;
    virtual void onTimeout();
    virtual void onDisconnect() = 0;

    protected:
    std::set<MessageType> whitelist, blacklist;

    private:
    MessageSocket msock;
    bool _running = true;
    bool _timeoutActive = false;
    TimePoint timeoutDeadline;
};