#include <gamemanager.h>

#include <algorithm>
#include <cassert>
#include <iostream>

GameJoinError GameManager::hostNewGame(const std::string &username, const Map &map, GameID &outGameID) {
    std::scoped_lock lk(mutex);
    assert(!findGameByPlayer(username));
    assert(&map != nullptr);
    
    outGameID = initNewGame(map);
    return joinGame(username, outGameID);
}

GameJoinError GameManager::joinGame(const std::string &username, GameID gameID) {
    std::scoped_lock lk(mutex);

    assert(!findGameByPlayer(username));

    if(games.find(gameID) == games.end())
        return GameJoinError::GAME_DOESNT_EXIST;

    Entry &entry = games[gameID];
    if(entry.ready())
        return GameJoinError::GAME_ALREADY_RUNNING;

    entry.players.push_back(username);
    playerGames[username] = gameID;

    if(entry.ready())
        entry.game = std::make_unique<Game>(gameID, *entry.map, entry.players);

    return GameJoinError::NO_ERROR;
}

GameJoinError GameManager::joinAnyGame(const std::string &username, GameID &outGameID) {
    std::scoped_lock lk(mutex);
    assert(!findGameByPlayer(username));

    if(joinableGames.empty()) {
        std::uniform_int_distribution<int> dist(0, Map::registry.size()-1);
        const auto &map = Map::registry[dist(rng)];
        joinableGames.insert(initNewGame(map));
    }
    auto id = *joinableGames.begin();
    auto result = joinGame(username, id);
    if(isGameReady(id))
        joinableGames.erase(id);

    if(result == GameJoinError::NO_ERROR)
        outGameID = id;
    return result;
}

void GameManager::leaveGame(const std::string &username) {
    std::scoped_lock lk(mutex);

    auto id = findGameByPlayer(username);
    assert(id);
    assert(games.find(id) != games.end());
    auto &entry = games[id];

    entry.players.erase(std::find(entry.players.begin(), entry.players.end(), username));
    //TODO: if game has not ended, then make the leaving player automatically surrender
    if(entry.players.empty()) {
        games.erase(id);
        if(joinableGames.find(id) != joinableGames.end())
            joinableGames.erase(id);
    }

    playerGames.erase(username);
}

GameID GameManager::initNewGame(const Map &map) {
    auto id = nextGameID++;
    games[id].map = &map;
    return id;
}

GameID GameManager::findGameByPlayer(const std::string &username) {
    std::scoped_lock lk(mutex);
    auto it = playerGames.find(username);
    if(it == playerGames.end())
        return 0;
    else
        return it->second;
}

bool GameManager::isGameReady(GameID id) {
    std::scoped_lock lk(mutex);
    auto it = games.find(id);
    if(it == games.end())
        return false;
    return it->second.ready();
}

Game &GameManager::getGame(GameID id) {
    std::scoped_lock lk(mutex);
    assert(isGameReady(id));
    return *games[id].game;
}

std::mutex &GameManager::getGameMutex(GameID id) {
    std::scoped_lock lk(mutex);
    assert(isGameReady(id));
    return games[id].gameMutex;
}