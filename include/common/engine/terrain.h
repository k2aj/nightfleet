#pragma once 

#include <string>
#include <engine/registry.h>

class TerrainType : public ContentType<TerrainType> {
    public:
    int movementCost = 10;

    TerrainType(const std::string &id);
};