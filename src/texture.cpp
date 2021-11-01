
#include "texture.h"
#include "debug.h"

#include <stb_image.h>
#include <vector>

Texture::Texture(const std::string& texturePath, GLenum target) : m_Path(texturePath), m_Target(target), m_LocalBuffer(nullptr)
{
    GLCall(glGenTextures(1, &m_TextureId));

    switch (target) {
        case GL_TEXTURE_2D:
            GenerateTexture2D();
            break;
        case GL_TEXTURE_2D_ARRAY:
            GenerateTexture2DArray();
            break;
        default:
            std::cout << "ERROR: Texture target format unsupported" << std::endl;
            break;
    }

}

Texture::~Texture()
{
    GLCall( glDeleteTextures(1, &m_TextureId) );
}

void Texture::GenerateTexture2DArray() // glActiveTexture?
{
    int channels = 4; // 4 desired channels for RGBA

    stbi_set_flip_vertically_on_load(true);
    m_LocalBuffer = stbi_load(m_Path.c_str(), &m_Width, &m_Height, &m_Channels, channels); // 0

    GLCall(glBindTexture(GL_TEXTURE_2D_ARRAY, m_TextureId)); // Bind without slot selection

    // set the texture wrapping parameters
    // GL_REPEAT necessary for meshing in future (out of bound indexing repeats texture)
    GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT)); // necessary?

    // removes texture bleeding and retains exact texture from atlas
    GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    // create 2D texture array from texture atlas image
    int tileWidth = 48, tileHeight = 48;
    int tilesX = 16, tilesY = 16;
    int imageCount = tilesX * tilesY;

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
                 tileWidth, tileHeight, imageCount, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // must manually specify each texture in array

    std::vector<unsigned char> tile(tileWidth * tileHeight * channels); // to store individual texture
    int tileSizeX = tileWidth * 4;
    int rowLen    = tilesX * tileSizeX;

    // iterate across each texture, filling in texture array
    for (int iy = 0; iy < tilesY; iy++)
    {
        for (int ix = 0; ix < tilesX; ix++)
        {
            unsigned char *ptr = m_LocalBuffer + iy*rowLen*tileHeight + ix*tileSizeX;
            for (int row = 0; row < tileHeight; row++)
                std::copy(ptr + row*rowLen, ptr + row*rowLen + tileSizeX,
                          tile.begin() + row*tileSizeX);


            int i = iy * tilesX + ix; // z offset of current texture
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                            0, 0, i,
                            tileWidth, tileHeight, 1,
                            GL_RGBA, GL_UNSIGNED_BYTE, tile.data());
        }
    }
    // might want to set max mipmap layer bc 48x48 textures (2^4 * 3 --> base + 4 mipmap layers)
    GLCall(glGenerateMipmap(GL_TEXTURE_2D_ARRAY));

    Unbind();

    if (m_LocalBuffer) {
        stbi_image_free(m_LocalBuffer);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    };
}

void Texture::GenerateTexture2D()
{
    stbi_set_flip_vertically_on_load(true);
    // desired channels originally 4
    m_LocalBuffer = stbi_load(m_Path.c_str(), &m_Width, &m_Height, &m_Channels, 4); // 0

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

void Texture::Bind(unsigned int slot)
{
    GLCall( glActiveTexture(GL_TEXTURE0 + slot) );
    GLCall( glBindTexture(m_Target, m_TextureId) );
}

void Texture::Unbind() const
{
    GLCall( glBindTexture(m_Target, 0) );
}