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
    void drawRectangle(
        const glm::vec2 &center, 
        const glm::vec2 &radii, 
        const glm::vec2 &rotation = glm::vec2(1,0)
    );
    void drawLine(
        const glm::vec2 &a,
        const glm::vec2 &b,
        float thickness
    );
    void mulColor(const glm::vec4 &value = {1,1,1,1});
    void addColor(const glm::vec4 &value = {0,0,0,0});

    void clear();
    void render(const glm::mat4 &projection);

    private:
    
    struct Vertex {
        glm::vec2 position;
        glm::u16vec2 uv;
        glm::u8vec4 mulColor, addColor;
    };

    std::vector<Vertex> vertices;
    GLuint vao, vbo;
    ShaderProgram shaderProgram;
    Atlas atlas;
    AtlasArea whiteImage;

    glm::u8vec4 _mulColor{255}, _addColor{0};
};