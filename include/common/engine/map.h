#pragma once

#include <vector>

#include <engine/field.h>
#include <engine/terrain.h>
#include <engine/unit.h>

struct Map {

    Field<const TerrainType *> terrain;
    std::vector<std::vector<Unit>> startingUnits;

    int playerCount() const;

    Map(glm::ivec2 size, int playerCount);
};