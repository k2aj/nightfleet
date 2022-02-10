#pragma once

#include <vector>
#include <string>
#include <utility>

#include <glad/glad.h>

class ShaderProgram {
    private:
    GLuint handle = 0;
    std::vector<GLuint> shaders;

    public:
    ShaderProgram(std::vector<std::pair<std::string,GLenum>> sources);
    ~ShaderProgram();

    void bind();
};