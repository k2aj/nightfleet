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
    int maxHealth = 50,
        armor = 5,
        evasion = 5,
        movementPointsPerTurn = 50,
        actionPointsPerTurn = 1,

        attackDamage = 30,
        attackPenetration = 20,
        attackAccuracy = 20,
        attackRange = 1;

    UnitType(const std::string &id);
};

struct Unit {

    const UnitType *type;
    int player, health, movementPoints = 0, actionPoints = 0;
    glm::ivec2 position{0};

    Unit();
    Unit(UnitType &type, int player, glm::ivec2 position);

    bool isAlive() const;
    void update(Game &game);

    void attack(Unit &other);
};

RxBuffer &operator>>(RxBuffer &rx, Unit &unit);
TxBuffer &operator<<(TxBuffer &tx, const Unit &unit);

