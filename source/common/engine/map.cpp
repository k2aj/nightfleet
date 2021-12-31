#include <engine/map.h>

#include <cassert>

Map::Map(glm::ivec2 size, int playerCount) : 
    terrain(size, &TerrainType::registry.getDefault()),
    startingUnits(playerCount)
{}

int Map::playerCount() const {
    return static_cast<int>(startingUnits.size());
}