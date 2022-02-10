#pragma once

#include <string>
#include <glm/vec2.hpp>
#include <engine/registry.h>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>

// workaround for not being able to #include <common/engine/game.h>
class Game;

class UnitType : public ContentType<UnitType> {
    public:

    // unit stats
    int maxHealth = 10,
        armor = 0,
        evasion = 0,
        movementPointsPerTurn = 5,
        actionPointsPerTurn = 1;

    UnitType(const std::string &id);
};

struct Unit {

    const UnitType *type;
    int player, health, movementPoints = 0, actionPoints = 0;
    glm::ivec2 position{0};

    Unit();
    Unit(UnitType &type, int player);

    bool isAlive() const;
    void update(Game &game);
};

RxBuffer &operator>>(RxBuffer &rx, Unit &unit);
TxBuffer &operator<<(TxBuffer &tx, const Unit &unit);

