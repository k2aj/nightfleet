#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glad/glad.h>

struct AtlasArea {
    glm::ivec2 offset, size;
};

class Atlas {

    public:

    Atlas();
    Atlas(const glm::ivec2 &minSize);

    AtlasArea allocate(const glm::ivec2 &minSize);
    AtlasArea load(const std::string &path);
    glm::ivec2 size() const;
    void attach(GLenum texUnit = GL_TEXTURE0);

    private:
    void expand();
    void createFreeBlocks(int log2BlkSize);

    GLuint texture;
    int log2Size;
    std::vector<std::vector<AtlasArea>> freeBlocks;
};