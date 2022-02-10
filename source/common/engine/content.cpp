#include <engine/content.h>

#include <engine/terrain.h>
#include <engine/unit.h>
#include <engine/map.h>

TerrainType
    space("space");

UnitType
    fighter("fighter");

Map
    emptyMap("Empty Map", glm::ivec2(32,18), 2, &space),
    kesslerSyndrome("Kessler Syndrome", glm::ivec2(32,18), 2, &space);

void initGameContent() {

    TerrainType::registry.add(space);

    UnitType::registry.add(fighter);

    Map::registry.add(emptyMap);
    Map::registry.add(kesslerSyndrome);
}