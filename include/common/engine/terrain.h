#pragma once 

#include <string>
#include <engine/registry.h>

class TerrainType : public ContentType<TerrainType> {
    public:

    TerrainType(const std::string &id);
};