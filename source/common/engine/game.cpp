#include <engine/game.h>

#include <cassert>

Game::Game(const Map &map) :
    terrain(map.terrain),
    units(terrain.size(), nullptr),
    playerUnitPositions(map.playerCount())
{
    //spawn starting units
    for(int player = 0; player < map.playerCount(); ++player)
        for(const auto &unit : map.startingUnits[player])
            spawn(std::make_shared<Unit>(unit));
}

int Game::playerCount() const {
    return static_cast<int>(playerUnitPositions.size());
}

int Game::currentPlayer() const {
    return _currentPlayer;
}

std::shared_ptr<const Unit> Game::unitAt(const glm::ivec2 &position) const {
    return units.get(position);
}

std::shared_ptr<Unit> Game::unitAt(const glm::ivec2 &position) {
    return units.get(position);
}

void Game::spawn(std::shared_ptr<Unit> unit) {
    assert(!unitAt(unit->position));
    assert(unit->player >= 0 && unit->player < playerCount());
    playerUnitPositions[unit->player].insert(unit->position);
    units.set(unit->position, unit);
}

void Game::endTurn() {
    for(auto &unitPos : playerUnitPositions[currentPlayer()])
        unitAt(unitPos)->update(*this);

   _currentPlayer = (currentPlayer() + 1) % playerCount();
}