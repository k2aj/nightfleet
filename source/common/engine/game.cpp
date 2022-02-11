#include <engine/game.h>

#include <cassert>
#include <numeric>

Game::Game() {}

Game::Game(GameID id, const Map &map, const std::vector<std::string> &playerUsernames) :
    terrain(map.terrain),
    units(terrain.size(), nullptr),
    playerUnitPositions(map.playerCount()),
    _id(id),
    playerUsernames(playerUsernames)
{
    assert(map.playerCount() == playerUsernames.size());
    //spawn starting units
    for(int player = 0; player < map.playerCount(); ++player)
        for(const auto &unit : map.startingUnits[player])
            spawn(std::make_shared<Unit>(unit));
}

int Game::playerCount() const {
    return static_cast<int>(playerUnitPositions.size());
}

const std::string &Game::currentPlayer() const {
    return playerUsernames[_currentPlayer];
}
GameID Game::id() const {
    return _id;
}

std::shared_ptr<const Unit> Game::unitAt(const glm::ivec2 &position) const {
    return units.getOr(position, {});
}

std::shared_ptr<Unit> Game::unitAt(const glm::ivec2 &position) {
    return units.getOr(position, {});
}

void Game::spawn(std::shared_ptr<Unit> unit) {
    assert(!unitAt(unit->position));
    assert(unit->player >= 0 && unit->player < playerCount());
    playerUnitPositions[unit->player].insert(unit->position);
    units.set(unit->position, unit);
}

void Game::endTurn() {
    for(auto &unitPos : playerUnitPositions[_currentPlayer])
        unitAt(unitPos)->update(*this);

   _currentPlayer = (_currentPlayer + 1) % playerCount();
}

void Game::makeMove(const Move &m) {
    switch(m.type) {

        case MoveType::MOVE_UNIT: {
            if(m.args.size() % 2 != 0)
                throw InvalidMoveError("Malformed move description.");
            for(int i=0; i+2<m.args.size(); i+=2)
                moveUnitOneTile(glm::ivec2(m.args[i], m.args[i+1]), glm::ivec2(m.args[i+2], m.args[i+3]));
        }
        break;

        case MoveType::ATTACK_UNIT: {
            if(m.args.size() != 4)
                throw InvalidMoveError("Malformed move description.");

            glm::ivec2 pAttacker(m.args[0], m.args[1]), pTarget(m.args[2], m.args[3]);
            auto attacker = unitAt(pAttacker), target = unitAt(pTarget);

            if(attacker == nullptr)
                throw InvalidMoveError("No attacking unit.");
            if(attacker->player != _currentPlayer)
                throw InvalidMoveError("Player does not own the unit.");
            if(target == nullptr)
                throw InvalidMoveError("No target.");
            if(attacker->actionPoints <= 0)
                throw InvalidMoveError("Not enough action points.");

            attacker->attack(*target);
            if(!target->isAlive()) {
                playerUnitPositions[target->player].erase(target->position);
                units.set(target->position, {});
            }
        }
        break;

        case MoveType::END_TURN:
            endTurn();
        break;

        default:
            throw InvalidMoveError("Not implemented.");
    }
}

void Game::moveUnitOneTile(const glm::ivec2 &from, const glm::ivec2 &to) {
    if(abs(from.x - to.x) + abs(from.y - to.y) != 1)
        throw InvalidMoveError("Discontinuous movement path.");

    auto unit = unitAt(from);

    if(unit == nullptr)
        throw InvalidMoveError("Tile does not contain a unit.");

    if(unit->player != _currentPlayer)
        throw InvalidMoveError("Player does not own the unit.");

    if(unit->movementPoints <= 0)
        throw InvalidMoveError("Movement points depleted.");

    if(unitAt(to))
        throw InvalidMoveError("Destination tile is occupied.");

    if(!terrain.inBounds(to))
        throw InvalidMoveError("Destination tile out of bounds.");
    
    unit->position = to;
    unit->movementPoints -= terrain.get(from)->movementCost;
    unit->movementPoints -= terrain.get(to)->movementCost;
    units.set(to, unit);
    units.set(from, {});
    playerUnitPositions[_currentPlayer].erase(from);
    playerUnitPositions[_currentPlayer].insert(to);
}

template<typename T>
void readContentTypeField(RxBuffer &rx, Field<const T*> &field) {
    auto width = rx.read<uint32_t>();
    auto height = rx.read<uint32_t>();
    field = Field<const T*>(glm::ivec2(width, height));
    for(uint32_t y=0; y<height; ++y)
        for(uint32_t x=0; x<width; ++x) {
            field.set(glm::ivec2(x,y), readContentType<T>(rx));
        }
}

RxBuffer &operator>>(RxBuffer &rx, Game &game) {

    rx >> game._id;

    rx >> game._currentPlayer;
    rx >> game.playerUsernames;
    auto playerCount = game.playerUsernames.size();

    readContentTypeField<TerrainType>(rx, game.terrain);

    auto unitCount = rx.read<uint32_t>();
    game.playerUnitPositions.resize(playerCount);

    for(auto i=0; i<unitCount; ++i) {
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

    // players
    tx << game._currentPlayer;
    tx << game.playerUsernames;

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