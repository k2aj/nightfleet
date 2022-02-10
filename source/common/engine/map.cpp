#include <engine/map.h>

#include <cassert>

Map::Map(const std::string &id, glm::ivec2 size, int playerCount, const TerrainType *filler) : 
    ContentType(id),
    terrain(size, filler),
    startingUnits(playerCount)
{}

Map::Map() : Map("", glm::ivec2(0,0), 0, nullptr) {}

int Map::playerCount() const {
    return static_cast<int>(startingUnits.size());
}