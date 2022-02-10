#include <engine/game.h>

#include <cassert>
#include <numeric>

Game::Game() {}

Game::Game(GameID id, const Map &map) :
    terrain(map.terrain),
    units(terrain.size(), nullptr),
    playerUnitPositions(map.playerCount()),
    _id(id)
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
GameID Game::id() const {
    return _id;
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

RxBuffer &operator>>(RxBuffer &rx, Game &game) {

    rx >> game._id;

    auto playerCount = rx.read<uint32_t>();
    rx >> game._currentPlayer;

    rx >> game.terrain;

    auto unitCount = rx.read<uint32_t>();
    game.playerUnitPositions.resize(playerCount);

    for(uint32_t i=0; i<unitCount; ++i) {
        auto unit = rx.read<Unit>();

        if(!game.units.inBounds(unit.position))
            throw std::out_of_range("Unit position out of range!");

        game.units.set(unit.position, std::make_shared<Unit>(unit));
        game.playerUnitPositions.at(unit.player).insert(unit.position);
    }

    return rx;
}
TxBuffer &operator<<(TxBuffer &tx, const Game &game) {

    tx << game._id;

    // # of players & current active player
    tx << game.playerCount() << game._currentPlayer;

    // terrain
    tx << game.terrain;
    
    // units
    uint32_t unitCount = std::transform_reduce(
        game.playerUnitPositions.begin(), 
        game.playerUnitPositions.end(), 
        0, 
        [](auto a, auto b){return a+b;}, 
        [](auto &v){return v.size();}
    );

    tx << unitCount;
    for(const auto &positionList : game.playerUnitPositions)
        for(auto position : positionList)
            tx << *game.unitAt(position);

    return tx;
}