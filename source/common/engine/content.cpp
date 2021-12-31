#include <engine/content.h>

#include <engine/terrain.h>
#include <engine/unit.h>

TerrainType
    space("space");

UnitType
    fighter("fighter");

void initGameContent() {
    TerrainType::registry.add(space);
    UnitType::registry.add(fighter);
}