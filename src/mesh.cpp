
#include "mesh.h"

// should pass VAO, shader, texture
// updateVBO(std::vector<texArrayVertex>&)
Mesh::Mesh(std::vector<BlockType>& blocks)
    : m_VBO(0), m_VAO(nullptr), m_Shader(nullptr), m_Texture(nullptr)
{};

Mesh::~Mesh()
{
    glDeleteBuffers(1, &m_VBO);
};

