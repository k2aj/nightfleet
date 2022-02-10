#include <dgl/shader.h>

#include <stdexcept>

ShaderProgram::ShaderProgram(std::vector<std::pair<std::string,GLenum>> sources) {

    handle = glCreateProgram();

    for(auto [sourceCode, shaderType] : sources) {

        GLuint shader = glCreateShader(shaderType);
        shaders.push_back(shader);

        const char *csrc = sourceCode.c_str();
        GLint csrcLen = sourceCode.length();
        glShaderSource(shader, 1, &csrc, &csrcLen);
        glCompileShader(shader);

        GLint compileStatus;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

        if(compileStatus != GL_TRUE) {
            GLint infoLogLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::string infoLog(infoLogLength, ' ');
            GLsizei dummy;
            glGetShaderInfoLog(shader, infoLog.size(), &dummy, &infoLog[0]);
            throw std::runtime_error(std::string("Failed to compile shader:\n") + infoLog);
        }

        glAttachShader(handle, shader);
    }

    glLinkProgram(handle);

    GLint linkStatus;
    glGetProgramiv(handle, GL_LINK_STATUS, &linkStatus);

    if(linkStatus != GL_TRUE) {
        GLint infoLogLength;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::string infoLog(infoLogLength, ' ');
        GLsizei dummy;
        glGetProgramInfoLog(handle, infoLog.size(), &dummy, &infoLog[0]);
        throw std::runtime_error(std::string("Failed to link shader program:\n") + infoLog);
    }

}
ShaderProgram::~ShaderProgram() {
    for(auto shader : shaders)
        glDeleteShader(shader);
    glDeleteProgram(handle);
}

void ShaderProgram::bind() {
    glUseProgram(handle);
}
