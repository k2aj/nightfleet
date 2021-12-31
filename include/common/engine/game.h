#pragma once

#include <memory>
#include <vector>
#include <set>

#include <glm/vec2.hpp>

#include <engine/field.h>
#include <engine/map.h>
#include <engine/unit.h>

struct IVec2Comparator {
    bool operator()(const glm::ivec2 &lhs, const glm::ivec2 &rhs) const {
        if(lhs.x < rhs.x) return true;
        else if(lhs.x > rhs.x) return false;
        else return lhs.y < rhs.y;
    }
};

class Game {

    public:

    Game(const Map &map);

    int playerCount() const;
    int currentPlayer() const;
    std::shared_ptr<const Unit> unitAt(const glm::ivec2 &position) const;
    std::shared_ptr<Unit> unitAt(const glm::ivec2 &position);

    void spawn(std::shared_ptr<Unit> unit);
    void endTurn();

    private:

    int _currentPlayer = 0;

    // we don't store a Map because maybe the terrain will get modified during the game
    Field<const TerrainType *> terrain;

    // there can only be one unit per terrain tile
    Field<std::shared_ptr<Unit>> units;

    std::vector<std::set<glm::ivec2, IVec2Comparator>> playerUnitPositions;
};