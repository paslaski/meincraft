
#include "ChunkMeshingSystem.h"
#include "components.h"

ChunkMeshingSystem::ChunkMeshingSystem()
{}

ChunkMeshingSystem::~ChunkMeshingSystem()
{}

void ChunkMeshingSystem::update(entt::registry& registry)
{
    // view all chunks (data stored in BlockComponents)
    auto chunkView = registry.view<BlockComponent>();
    for (auto chunk : chunkView)
    {
        // only have to update mesh if blocks have changed
        if (registry.get<BlockComponent>(chunk).hasChanged)
            constructMesh(chunk, registry);
    }
}

void ChunkMeshingSystem::constructMesh(entt::entity& chunk, entt::registry& registry)
{
    // retrieve refs to block data & vertex storage
    BlockComponent& blocks = registry.get<BlockComponent>(chunk);
    std::vector<texArrayVertex>& vertices = registry.get<MeshComponent>(chunk).chunkVertices;
    glm::vec3& pos = registry.get<PositionComponent>(chunk).pos;

    vertices.clear(); // delete old vertex data

    float uvCoords[] = {
            0.0f, 0.0f, // first triangle on face
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.0f, 1.0f, // second triangle on face
            1.0f, 1.0f,
            1.0f, 0.0f
    };
    // aligns direction with faces 0-5 in loop
    // WEST, DOWN, NORTH, EAST, UP, SOUTH
    Direction dir[] = {
            WEST, DOWN, NORTH, EAST, UP, SOUTH
    };

    // create new vertex data for mesh from scratch
    // position.pos refers to (i, j, k) = (0, 0, 0) in world coordinates
    // i, j, k refer to block position in chunk coordinates
    for (int k = 0; k < CHUNK_SIZE; k++)
        for (int j = 0; j < CHUNK_SIZE; j++)
            for (int i = 0; i < CHUNK_SIZE; i++)
                for (int face = 0; face < 6; face++) // 6 faces for each cube
                    for (int v = 0; v < 6; v++) { // 6 vertices for each face
                        int dim = face % 3;
                        // follows UV coordinates of texture across 2D face surface
                        // also aligns UV with position in world space
                        float xVertOffset, yVertOffset, zVertOffset;
                        if (dim == 0) {
                            xVertOffset = 0.0f;
                            yVertOffset = uvCoords[2*v + 1];
                            zVertOffset = uvCoords[2*v];
                        } else if (dim == 1) {
                            xVertOffset = uvCoords[2*v];
                            yVertOffset = 0.0f;
                            zVertOffset = uvCoords[2*v + 1];
                        } else { // dim == 2
                            xVertOffset = uvCoords[2*v];
                            yVertOffset = uvCoords[2*v + 1];
                            zVertOffset = 0.0f;
                        }
                        // offsets by 1 in x, y, and z direction to create parallel faces
                        float xFaceOffset = (face == 3) ? 1.0f : 0.0f;
                        float yFaceOffset = (face == 4) ? 1.0f : 0.0f;
                        float zFaceOffset = (face == 5) ? 1.0f : 0.0f;
                        // need to support different textures by side
                        // current dim scheme (face = 0 through 5)
                        // WEST, DOWN, NORTH, EAST, UP, SOUTH
                        vertices.emplace_back(
                                //                         mesh corner + block corner + UV + 0/1
                                static_cast<GLfloat>(pos.x + i + xVertOffset + xFaceOffset),
                                static_cast<GLfloat>(pos.y + j + yVertOffset + yFaceOffset),
                                static_cast<GLfloat>(pos.z + k + zVertOffset + zFaceOffset),
                                uvCoords[v*2], uvCoords[v*2 + 1],
                                static_cast<GLfloat>(sideLookup(blocks.at(i, j, k), dir[face]))
                        );
                    }

    // note that new mesh was constructed based on changes
    // pretty sure this is an lvalue so this works
    registry.get<BlockComponent>(chunk).hasChanged = false;
    registry.get<MeshComponent>(chunk).mustUpdateBuffer = true;
}

void ChunkMeshingSystem::greedyMesh(entt::entity &chunk, entt::registry &registry)
{
    // retrieve refs to block data & vertex storage
    BlockComponent& blocks = registry.get<BlockComponent>(chunk);
    std::vector<texArrayVertex>& vertices = registry.get<MeshComponent>(chunk).chunkVertices;
    glm::vec3& pos = registry.get<PositionComponent>(chunk).pos;

    vertices.clear(); // delete old vertex data

    // aligns direction with faces 0-5 in loop
    // WEST, DOWN, NORTH, EAST, UP, SOUTH
    Direction dir[] = {
            WEST, DOWN, NORTH, EAST, UP, SOUTH
    };


    // greedy meshing algorithm


    // note that new mesh was constructed based on changes
    // pretty sure this is an lvalue so this works
    registry.get<BlockComponent>(chunk).hasChanged = false;
    registry.get<MeshComponent>(chunk).mustUpdateBuffer = true;
}

// appends a face to the texArrayVertex vector (two triangles/6 vertices, "quad" for short)
void ChunkMeshingSystem::quad(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4,
                              BlockType block, Direction dir, std::vector<texArrayVertex>& vertices) {
    float uvCoords[] = {
            0.0f, 0.0f, // first triangle on face
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.0f, 1.0f, // second triangle on face
            1.0f, 1.0f,
            1.0f, 0.0f
    };
}

