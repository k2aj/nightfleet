#include <dgl/debug.h>

#include <glad/glad.h>
#include <map>

using std::endl;
using std::string;

std::map<GLenum, string> debugSourceToStr = {
    {GL_DEBUG_SOURCE_API, "OpenGL API"},
    {GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Window API"},
    {GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader compiler"},
    {GL_DEBUG_SOURCE_THIRD_PARTY, "Third party"},
    {GL_DEBUG_SOURCE_APPLICATION, "NightFleet"},
    {GL_DEBUG_SOURCE_OTHER, "Other"}
};

std::map<GLenum, string> debugTypeToStr = {
    {GL_DEBUG_TYPE_ERROR, "type error"},
    {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "deprecated behavior"},
    {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "undefined behavior"},
    {GL_DEBUG_TYPE_PORTABILITY, "portability issue"},
    {GL_DEBUG_TYPE_PERFORMANCE, "performance issue"},
    {GL_DEBUG_TYPE_MARKER, "cmd stream marker"},
    {GL_DEBUG_TYPE_PUSH_GROUP, "group pushing"},
    {GL_DEBUG_TYPE_OTHER, "other"}
};

std::map<GLenum, string> severityToStr = {
    {GL_DEBUG_SEVERITY_HIGH, "!!!HIGH!!!"},
    {GL_DEBUG_SEVERITY_MEDIUM, "MEDIUM"},
    {GL_DEBUG_SEVERITY_LOW, "low"},
    {GL_DEBUG_SEVERITY_NOTIFICATION, "info"}
};

void APIENTRY openglMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {

    std::ostream &out = *((std::ostream *)userParam);

    out << "[GL/";
    out << debugSourceToStr[source] << "] ";
    out << "(" << severityToStr[severity] << ") ";
    out << debugTypeToStr[type] << ": " << message;
    out << endl; 
}

void initOpenGLDebugOutput(std::ostream &out) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&openglMessageCallback, &out);
}