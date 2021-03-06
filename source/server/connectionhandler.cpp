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
    size_t knownMoveCount;

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
                    knownMoveCount = 0;
                    fsm = INGAME;
                }
            }
            break;

            case INGAME: {
                std::scoped_lock lk(server.gameManager.getGameMutex(gameID));
                auto &globalMoves = server.gameManager.getMoveList(gameID);
                if(globalMoves.size() > knownMoveCount) {
                    GameIncrementalSync sync;
                    for(; knownMoveCount < globalMoves.size(); ++knownMoveCount)
                        sync.moveList.push_back(globalMoves[knownMoveCount]);
                    sendIncrementalSync(sync);
                }
            }

            default: break;
        }
        if(
            server.status() == ServerStatus::FAST_SHUTDOWN ||
            (server.status() == ServerStatus::SLOW_SHUTDOWN && fsm == IDLE)    
        ) {
            haltReason = "Server shutting down.";
            cleanupAndHalt();
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
        if(error == GameJoinError::NO_ERROR) {
            fsm = AWAITING_GAME;
            sendHostGameAck({gameID});
        } else
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
            if(fsm == INGAME)
                sendLeaveGameRequest({});
            server.gameManager.leaveGame(username);
            fsm = IDLE;
        } else
            throw ProtocolError("Unexpected LeaveGameRequest");
    }

    void onIncrementalSync(const GameIncrementalSync &sync) override {
        if(fsm != INGAME)
            return;

        std::scoped_lock lk(server.gameManager.getGameMutex(gameID));
        auto &game = server.gameManager.getGame(gameID);
        auto &globalMoves = server.gameManager.getMoveList(gameID);
        
        for(auto move : sync.moveList)
            try {
                if(game.currentPlayer() != username)
                    throw InvalidMoveError("Player attempted to move when it was not their turn.");
                if(move.type == MoveType::FORCED_SURRENDER)
                    throw InvalidMoveError("Client is not allowed to send force surrender.");
                game.makeMove(move);
                globalMoves.push_back(move);
                ++knownMoveCount;
            } catch (InvalidMoveError &e) {
                throw ProtocolError("Invalid move: " + std::string(e.what()));
            }
        
    }

    void cleanupAndHalt() {
        if(!username.empty()) {
            if(fsm == AWAITING_GAME || fsm == INGAME)
                server.gameManager.leaveGame(username);
            server.userManager.logout(username);
        }
        halt();
    }

    void onProtocolError(const ProtocolError &e) override {
        haltReason = std::string("ProtocolError: ")+std::string(e.what());
        cleanupAndHalt();
    }

    void onTimeout() override {
        haltReason = "timed out";
        cleanupAndHalt();
    }
    
    void onDisconnect() override {
        haltReason = "closed by remote host";
        cleanupAndHalt();
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
        if(!entity.isRunning())
            break;
        entity.onUpdate(dt);

        sleep(10ms);
    }

    std::cerr << "Connection terminated (sockfd="<<sockfd<<")";
    if(!entity.haltReason.empty())
        std::cerr << ", reason: " << entity.haltReason;
    std::cerr << std::endl;
}

