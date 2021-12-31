#pragma once 

#include <string>
#include <engine/registry.h>

class TerrainType {
    public:
    const std::string id;

    TerrainType(const std::string &id);

    static Registry<TerrainType> registry;
};