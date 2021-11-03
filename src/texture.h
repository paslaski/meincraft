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

private:
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
    GLfloat xWorldPos; GLfloat yWorldPos; GLfloat zWorldPos;
    GLfloat uTexCoord; GLfloat vTexCoord; GLfloat pictureNum;

    texArrayVertex(GLfloat xPos, GLfloat yPos, GLfloat zPos,
                   GLfloat uCoord, GLfloat vCoord, GLfloat picNum)
            : xWorldPos(xPos), yWorldPos(yPos), zWorldPos(zPos),
              uTexCoord(uCoord), vTexCoord(vCoord), pictureNum(picNum)
    {};
};
