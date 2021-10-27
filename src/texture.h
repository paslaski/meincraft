#pragma once

#include <string>

class Texture
{
public:
    Texture(const std::string& texturePath);
    ~Texture();
    void Bind(unsigned int slot = 0) const;
    void Unbind() const;
private:
    unsigned int m_TextureId;

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    unsigned char* m_LocalBuffer;
    int m_Width, m_Height, m_Channels;
};