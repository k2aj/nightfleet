#include <engine/unit.h>

UnitType::UnitType(const std::string &id) :
    id(id)
{}

Unit::Unit(const UnitType &type, int player) : 
    player(player),
    type(&type),
    health(type.maxHealth)
{}

bool Unit::isAlive() const {
    return health > 0;
}

void Unit::update(Game &game) {
    actionPoints = type->actionPointsPerTurn;
    movementPoints = type->movementPointsPerTurn;
}