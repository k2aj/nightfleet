#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <dgl/atlas.h>
#include <dgl/shader.h>

class SpriteRenderer {

    public:

    SpriteRenderer();
    ~SpriteRenderer();

    AtlasArea loadImage(const std::string &path);
    void drawImage(
        AtlasArea image, 
        const glm::vec2 &center, 
        const glm::vec2 &radii, 
        const glm::vec2 &rotation = glm::vec2(1,0)
    );

    void clear();
    void render(const glm::mat4 &projection);

    private:
    
    struct Vertex {
        glm::vec2 position;
        glm::u16vec2 uv;
    };

    std::vector<Vertex> vertices;
    GLuint vao, vbo;
    ShaderProgram shaderProgram;
    Atlas atlas;
};