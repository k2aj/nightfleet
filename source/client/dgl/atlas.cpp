#include <dgl/atlas.h>

#include <stb_image.h>
#include <iostream>

GLuint allocateRgbaTexture(const glm::ivec2 &size) {
    glActiveTexture(GL_TEXTURE0);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
    return texture;
}

Atlas::Atlas() : Atlas(glm::ivec2(1024,1024)) {}

unsigned int ceilLog2(unsigned int x) {
    if(x) 
        --x;
    unsigned int result = 0;
    while(x) {
        ++result;
        x >>= 1;
    };
    return result;
}

Atlas::Atlas(const glm::ivec2 &minSize) : 
    log2Size(ceilLog2(std::max(minSize.x, minSize.y)))
{
    texture = allocateRgbaTexture(size());
    freeBlocks.resize(log2Size+1);
    freeBlocks.at(log2Size).push_back({{0,0}, size()});
}

AtlasArea Atlas::allocate(const glm::ivec2 &minSize) {
    int log2NeededSize = ceilLog2(std::max(minSize.x, minSize.y));

    if(log2NeededSize >= freeBlocks.size() || freeBlocks[log2NeededSize].empty())
        createFreeBlocks(log2NeededSize);

    auto result = freeBlocks[log2NeededSize].back();
    std::cerr << "Min size: " << minSize.x << ' ' << minSize.y << std::endl;
    std::cerr << "Block: " << result.offset.x << ' ' << result.offset.y << ' ' << result.size.x << ' ' << result.size.y << std::endl;
    freeBlocks[log2NeededSize].pop_back();
    return result;
}

AtlasArea Atlas::load(const std::string &path) {

    int width, height, comp;
    unsigned char *image = stbi_load(path.c_str(), &width, &height, &comp, STBI_rgb_alpha);

    if(image == nullptr)
        return AtlasArea{{0,0}, {1,1}};
    
    auto block = allocate(glm::ivec2(width, height));
    block.size = {width, height};

    attach();
    std::cerr << "glTexSubImage2D params: " << block.offset.x << ' ' << block.offset.y <<' ' << width << ' ' << height << std::endl; 
    glTexSubImage2D(GL_TEXTURE_2D, 0, block.offset.x, block.offset.y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);

    stbi_image_free(image);

    return block;
}

void Atlas::attach(GLenum texUnit) {
    glActiveTexture(texUnit);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void Atlas::createFreeBlocks(int log2BlkSize) {

    while(log2BlkSize >= log2Size)
        expand();

    if(!freeBlocks[log2BlkSize].empty())
        return;
    
    if(freeBlocks[log2BlkSize+1].empty()) {
        if(log2BlkSize+1 >= log2Size)
            expand();
        createFreeBlocks(log2BlkSize+1);
    }

    auto block = freeBlocks[log2BlkSize+1].back();
    freeBlocks[log2BlkSize+1].pop_back();
    auto newBlockSize = block.size/glm::ivec2(2);

    for(int x=0; x<=1; ++x)
        for(int y=0; y<=1; ++y)
            freeBlocks[log2BlkSize].push_back({block.offset + newBlockSize*glm::ivec2(x,y), newBlockSize});
}

void Atlas::expand() {
    auto newSize = size()*2;
    auto newTexture = allocateRgbaTexture(newSize);
    glCopyImageSubData(
        texture, GL_TEXTURE_2D, 0,
        0, 0, 0,
        newTexture, GL_TEXTURE_2D, 0,
        0, 0, 0,
        size().x, size().y, 1
    );
    glDeleteTextures(1, &texture);
    texture = newTexture;
    
    freeBlocks.back().push_back({{0,size().y}, size()});
    freeBlocks.back().push_back({{size().x,0}, size()});
    freeBlocks.back().push_back({size(),size()});
    freeBlocks.push_back({});
    ++log2Size;
    std::cerr << "Size after expand = " << size().x << ' ' << size().y << std::endl;
}

glm::ivec2 Atlas::size() const {
    return glm::ivec2(1<<log2Size, 1<<log2Size);
}