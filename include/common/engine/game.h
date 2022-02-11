#pragma once

#include <memory>
#include <vector>
#include <set>
#include <mutex>

#include <glm/vec2.hpp>

#include <engine/field.h>
#include <engine/map.h>
#include <engine/unit.h>
#include <engine/move.h>

struct IVec2Comparator {
    bool operator()(const glm::ivec2 &lhs, const glm::ivec2 &rhs) const {
        if(lhs.x < rhs.x) return true;
        else if(lhs.x > rhs.x) return false;
        else return lhs.y < rhs.y;
    }
};

typedef int64_t GameID;

class Game : public MoveAcceptor {

    friend RxBuffer &operator>>(RxBuffer &rx, Game &game);
    friend TxBuffer &operator<<(TxBuffer &tx, const Game &game);

    public:

    Game();
    Game(GameID id, const Map &map, const std::vector<std::string> &playerUsernames);

    int playerCount() const;
    const std::string &currentPlayer() const;
    GameID id() const;
    std::shared_ptr<const Unit> unitAt(const glm::ivec2 &position) const;
    std::shared_ptr<Unit> unitAt(const glm::ivec2 &position);

    void spawn(std::shared_ptr<Unit> unit);
    void endTurn();

    void makeMove(const Move &) override;

    // we don't store a Map because maybe the terrain will get modified during the game
    Field<const TerrainType *> terrain;

    // there can only be one unit per terrain tile
    Field<std::shared_ptr<Unit>> units;

    private:

    void moveUnitOneTile(const glm::ivec2 &from, const glm::ivec2 &to);

    int _currentPlayer = 0;
    GameID _id;

    std::vector<std::set<glm::ivec2, IVec2Comparator>> playerUnitPositions;
    std::vector<std::string> playerUsernames;
};

RxBuffer &operator>>(RxBuffer &rx, Game &game);
TxBuffer &operator<<(TxBuffer &tx, const Game &game);