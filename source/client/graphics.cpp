#include <graphics.h>

#include <glm/gtc/type_ptr.hpp>

std::string vertexShaderSource = R"glsl(

        #version 430 compatibility

        layout(location=0) in vec2 position;
        layout(location=1) in vec2 uv;

        layout(location=1) uniform vec2 atlasSize;
        layout(location=2) uniform mat4 projection;

        out vec2 vUv;

        void main() {
            gl_Position = projection * vec4(position, 0.5, 1.0);
            vUv = uv / atlasSize;
        }
    )glsl",

    fragmentShaderSource = R"glsl(

        #version 430 compatibility

        in vec2 vUv;
        layout(location=0) uniform sampler2D atlas;
        out vec4 fragColor;
        
        void main() {
            fragColor = texture(atlas, vUv);
        }
    )glsl";

enum AttributeLocations {
    A_POSITION = 0,
    A_UV = 1
};
enum UniformLocations {
    U_ATLAS = 0,
    U_ATLAS_SIZE = 1,
    U_PROJECTION_MATRIX = 2
};
    
SpriteRenderer::SpriteRenderer() : 
    shaderProgram({
        {vertexShaderSource,   GL_VERTEX_SHADER},
        {fragmentShaderSource, GL_FRAGMENT_SHADER}
    })
{
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(A_POSITION);
    glVertexAttribPointer(A_POSITION, 2, GL_FLOAT, false, sizeof(Vertex), (const void*) offsetof(Vertex, position));

    glEnableVertexAttribArray(A_UV);
    glVertexAttribPointer(A_UV, 2, GL_UNSIGNED_SHORT, false, sizeof(Vertex), (const void*) offsetof(Vertex, uv));
}
SpriteRenderer::~SpriteRenderer() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

AtlasArea SpriteRenderer::loadImage(const std::string &path) {
    return atlas.load(path);
}

const glm::vec2 lkpPositions[] = {
    glm::vec2(-1,-1),
    glm::vec2( 1,-1),
    glm::vec2( 1, 1),
    glm::vec2(-1,-1),
    glm::vec2( 1, 1),
    glm::vec2(-1, 1)
};
const glm::ivec2 lkpUvs[] = {
    glm::ivec2(0,1),
    glm::ivec2(1,1),
    glm::ivec2(1,0),
    glm::ivec2(0,1),
    glm::ivec2(1,0),
    glm::ivec2(0,0)
};

glm::vec2 cmul(const glm::vec2 &a, const glm::vec2 &b) {
    return glm::vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

void SpriteRenderer::drawImage(
    AtlasArea image, 
    const glm::vec2 &center, 
    const glm::vec2 &radii, 
    const glm::vec2 &rotation
) {
    for(int i=0; i<6; ++i)
        vertices.push_back({
            center + cmul(radii * lkpPositions[i], rotation),
            image.offset + image.size*lkpUvs[i]
        });
}

void SpriteRenderer::clear() {
    vertices.clear();
}

#include <iostream>

void SpriteRenderer::render(const glm::mat4 &projection) {

    // Upload data to GPU
    if(!vertices.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STREAM_DRAW);
    }

    // Setup necessary OpenGL settings
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind VAO & shader program
    glBindVertexArray(vao);
    shaderProgram.bind();

    // Set values of uniform variables
    atlas.attach(GL_TEXTURE0);
    glUniform1i(U_ATLAS, 0);
    glUniform2f(U_ATLAS_SIZE, atlas.size().x, atlas.size().y);
    glUniformMatrix4fv(U_PROJECTION_MATRIX, 1, false, glm::value_ptr(projection));

    // draw
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}