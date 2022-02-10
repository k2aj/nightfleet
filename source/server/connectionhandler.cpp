#include <connectionhandler.h>

#include <iostream>

#include <network/message.h>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>
#include <network/protocol.h>
#include <util/time.h>
#include <scope_guard.h>

class NFServerProtocolEntity : public NFProtocolEntity {
    private:

    enum {
        DISCONNECTED,
        AWAITING_LOGIN,
        IDLE,
        AWAITING_GAME,
        INGAME
    } fsm = DISCONNECTED;

    Server &server;
    std::string username;
    GameID gameID = 0;

    public:
    std::string haltReason;

    NFServerProtocolEntity(int sockfd, Server &server) : 
        NFProtocolEntity(sockfd), 
        server(server) 
    {}


    void onVersionHandshake(const Version &version) override {

        blacklist.insert(MessageType::VERSION);
        whitelist = {MessageType::LOGIN_REQUEST};

        if(!applicationVersion.isCompatibleWith(version))
            halt();

        fsm = AWAITING_LOGIN;
    }

    void onLoginRequest(const LoginRequest &credentials) override {

        auto response = server.userManager.tryLogin(credentials);
        sendLoginResponse(response);

        if(response == LoginResponse::OK) {
            username = credentials.username;
            blacklist.insert(MessageType::LOGIN_REQUEST);
            whitelist.clear();
            fsm = IDLE;
        }
    }

    void onUpdate(const Duration &dt) override {
        switch(fsm) {
            case AWAITING_GAME: {
                if(server.gameManager.isGameReady(gameID)) {
                    std::scoped_lock lk(server.gameManager.getGameMutex(gameID));
                    sendFullSync(server.gameManager.getGame(gameID));
                    fsm = INGAME;
                }
            }
            break;

            default: break;
        }
    }

    void onHostGameRequest(const HostGameRequest &request) override {

        if(fsm != IDLE) return;

        if(server.status() != ServerStatus::RUNNING) {
            sendGameJoinError(GameJoinError::SERVER_SHUTTING_DOWN);
            halt();
        }

        if(server.gameManager.findGameByPlayer(username))
            throw ProtocolError("User is already in game.");

        auto error = server.gameManager.hostNewGame(username, *request.map, gameID);
        if(error == GameJoinError::NO_ERROR)
            fsm = AWAITING_GAME;
        else
            sendGameJoinError(error);
    }

    void onJoinGameRequest(const JoinGameRequest &request) override {

        if(fsm != IDLE) return;

        if(server.status() != ServerStatus::RUNNING) {
            sendGameJoinError(GameJoinError::SERVER_SHUTTING_DOWN);
            halt();
        }

        if(server.gameManager.findGameByPlayer(username))
            throw ProtocolError("User is already in game.");

        GameJoinError error;

        if(request.gameID == JoinGameRequest::JOIN_ANY)
            error = server.gameManager.joinAnyGame(username, gameID);
        else {
            error = server.gameManager.joinGame(username, request.gameID);
            if(error == GameJoinError::NO_ERROR) 
                gameID = request.gameID;
        }

        if(error == GameJoinError::NO_ERROR)
            fsm = AWAITING_GAME;
        else
            sendGameJoinError(error);
    }

    void onLeaveGameRequest(const LeaveGameRequest &request) override {
        if(fsm == AWAITING_GAME || fsm == INGAME) {
            server.gameManager.leaveGame(username);
            fsm = IDLE;
        } else
            throw ProtocolError("Unexpected LeaveGameRequest");
    }

    void onProtocolError(const ProtocolError &e) override {
        haltReason = std::string("ProtocolError: ")+std::string(e.what());
        halt();
    }

    void onTimeout() override {
        haltReason = "timed out";
        halt();
    }
    
    void onDisconnect() override {
        haltReason = "closed by remote host";
        halt();
    }
};

void handleConnection(int sockfd, Server *server) {

    TimePoint t0 = Clock::now();

    std::cerr << "Connection accepted (sockfd="<<sockfd<<")" << std::endl;
    scope_exit();

    NFServerProtocolEntity entity(sockfd, *server);
    entity.onInit();

    while(entity.isRunning()) {

        TimePoint t1 = Clock::now();
        Duration dt = t1 - t0;
        t0 = t1;

        entity.runNetworkEvents();
        entity.onUpdate(dt);

        sleep(10ms);
    }

    std::cerr << "Connection terminated (sockfd="<<sockfd<<")";
    if(!entity.haltReason.empty())
        std::cerr << ", reason: " << entity.haltReason;
    std::cerr << std::endl;
}

