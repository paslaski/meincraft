#pragma once

#include <string>

class Texture
{
public:
    Texture() = default;
    Texture(const std::string& texturePath);
    ~Texture();

//    // delete copy constructor and assignment operators
//    Texture(const Texture& texture) = delete; // copy constructor
//    Texture& operator=( const Texture & ) = delete; // copy assignment operator
//    // enable move constructor and assignment operators
//    Texture(Texture&& texture) = default;
//    Texture& operator=( Texture&& ) = default;

    void Bind(unsigned int slot = 0);
    void Unbind() const;
private:
    unsigned int m_TextureId;

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    unsigned char* m_LocalBuffer;
    int m_Width, m_Height, m_Channels;
};