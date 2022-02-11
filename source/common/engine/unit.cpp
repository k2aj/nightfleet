#include <engine/unit.h>

UnitType::UnitType(const std::string &id) :
    ContentType(id)
{}

Unit::Unit() {}
Unit::Unit(UnitType &type, int player, glm::ivec2 position) : 
    player(player),
    type(&type),
    health(type.maxHealth),
    actionPoints(type.actionPointsPerTurn),
    movementPoints(type.movementPointsPerTurn),
    position(position)
{}

bool Unit::isAlive() const {
    return health > 0;
}

void Unit::update(Game &game) {
    actionPoints = type->actionPointsPerTurn;
    movementPoints = type->movementPointsPerTurn;
}

void Unit::attack(Unit &other) {
    assert(actionPoints > 0);

    int totalDamage = 
        type->attackDamage * type->attackPenetration * type->attackAccuracy /
        ((type->attackPenetration+other.type->armor) * (type->attackAccuracy+other.type->evasion));

    --actionPoints;
    other.health = std::max(0, other.health-totalDamage);
}

RxBuffer &operator>>(RxBuffer &rx, Unit &unit) {
    unit.type = readContentType<UnitType>(rx);
    return (rx >> unit.player >> unit.health >> unit.movementPoints >> unit.actionPoints >> unit.position.x >> unit.position.y);
}
TxBuffer &operator<<(TxBuffer &tx, const Unit &unit) {
    return (tx << unit.type << unit.player << unit.health << unit.movementPoints << unit.actionPoints << unit.position.x << unit.position.y);
}