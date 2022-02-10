#pragma once

#include <engine/map.h>
#include <engine/game.h>
#include <network/protocol.h>
#include <map>
#include <set>
#include <mutex>
#include <memory>
#include <random>

/// This class is thread-safe
class GameManager {
    public:

    GameJoinError hostNewGame(const std::string &username, const Map &map, GameID &outGameID);
    GameJoinError joinGame(const std::string &username, GameID gameID);
    GameJoinError joinAnyGame(const std::string &username, GameID &outGameID);
    void leaveGame(const std::string &username);
    GameID findGameByPlayer(const std::string &username);
    bool isGameReady(GameID id);
    std::mutex &getGameMutex(GameID id);

    /** Note: the returned game object is NOT thread safe
     *  To ensure thread safety you need to lock the mutex obtained 
     *  from getGameMutex() before doing anything with returned game object
     */
    Game &getGame(GameID id);

    private:
    /// This method is NOT thread-safe
    GameID initNewGame(const Map &map);

    std::recursive_mutex mutex;
    std::default_random_engine rng;
    GameID nextGameID = 1;
    std::map<std::string, GameID> playerGames;

    struct Entry {
        const Map *map;
        std::unique_ptr<Game> game;
        std::mutex gameMutex;
        std::vector<std::string> players;

        bool ready() const {
            return players.size() == map->playerCount();
        }
    };
    std::map<GameID, Entry> games;
    std::set<GameID> joinableGames;
};