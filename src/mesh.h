#pragma once

#include <memory>
#include <vector>

#include "shader.h"
#include "texture.h"
#include "block.h"

class Mesh {
public:
    Mesh(std::vector<BlockType>& blocks);
    ~Mesh();
    // disable copying? or just have a mesh component by a std::unique_ptr<Mesh>
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void Update(std::vector<BlockType> blocks);

private:
    unsigned int m_VBO;
    std::shared_ptr<unsigned int> m_VAO;
    std::shared_ptr<Shader> m_Shader;
    std::shared_ptr<Texture> m_Texture;
};

