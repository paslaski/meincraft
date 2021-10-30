
#include "texture.h"
#include "debug.h"

#include <stb_image.h>

Texture::Texture(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);
    // desired channels originally 4
    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);
    GLCall(glGenTextures(1, &m_TextureId));
    GLCall(glBindTexture(GL_TEXTURE_2D, m_TextureId)); // Bind without slot selection

    // set the texture wrapping parameters
    // GL_REPEAT necessary for meshing in future (out of bound indexing repeats texture)
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    // removes texture bleeding and retains exact texture from atlas
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
    GLCall(glGenerateMipmap(GL_TEXTURE_2D));

    Unbind();

    if (m_LocalBuffer) {
        stbi_image_free(m_LocalBuffer);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    };
}

Texture::~Texture()
{
    GLCall( glDeleteTextures(1, &m_TextureId) );
}

void Texture::Bind(unsigned int slot)
{
    GLCall( glActiveTexture(GL_TEXTURE0 + slot) );
    GLCall( glBindTexture(GL_TEXTURE_2D, m_TextureId) );
}

void Texture::Unbind() const
{
    GLCall( glBindTexture(GL_TEXTURE_2D, 0) );
}