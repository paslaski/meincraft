#pragma once

#include <string>
#include <glad/glad.h>
#include <unordered_map>
#include "block.h"

class Texture
{
public:
    Texture() = default;
    Texture(const std::string& texturePath, GLenum target);
    ~Texture();

    void Bind(unsigned int slot = 0);
    void Unbind() const;

    void GenerateTexture2D();
    void GenerateTexture2DArray();

//    static float indexOf(BlockType block) { return blockIndexTable[block]; }

private:
//    static std::unordered_map<BlockType, float> blockIndexTable;

    unsigned int m_TextureId;
    GLenum m_Target;

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    unsigned char* m_LocalBuffer;
    int m_Width, m_Height, m_Channels;
    std::string m_Path;
};

struct texArrayVertex
{
    GLfloat worldPos[3];
    GLfloat textureCoords[3];
};

//// supports limited list for now
//std::unordered_map<BlockType, float> Texture::blockIndexTable =
//        {
//                {AIR, NULL}, // should never be drawn
//                {}
//        };
