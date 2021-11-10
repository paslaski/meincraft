
#include "ChunkMeshingSystem.h"
#include "Components.h"
#include <iostream>

ChunkMeshingSystem::ChunkMeshingSystem()
{}

ChunkMeshingSystem::~ChunkMeshingSystem()
{}

void ChunkMeshingSystem::update(entt::registry& registry)
{
    // view all chunks (data stored in BlockComponents)
    auto chunkView = registry.view<ChunkComponent>();
    for (auto chunk : chunkView)
    {
        // only have to update mesh if blocks have changed
        if (registry.get<ChunkComponent>(chunk).hasChanged)
            greedyMesh(chunk, registry);
    }
}

void ChunkMeshingSystem::constructMesh(entt::entity& chunk, entt::registry& registry)
{
//    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    // retrieve refs to block data & vertex storage
    ChunkComponent& blocks = registry.get<ChunkComponent>(chunk);
    std::vector<texArrayVertex>& vertices = registry.get<MeshComponent>(chunk).chunkVertices;
    glm::vec3& pos = registry.get<PositionComponent>(chunk).pos;

    vertices.clear(); // delete old vertex data

    // aligns direction with faces 0-5 in loop
    // WEST, DOWN, NORTH, EAST, UP, SOUTH
    Direction dir[] = {
            WEST, DOWN, NORTH, EAST, UP, SOUTH
    };

    // create new vertex data for mesh from scratch
    // position.pos refers to (i, j, k) = (0, 0, 0) in world coordinates
    // i, j, k refer to block position in chunk coordinates
    for (int k = 0; k < CHUNK_WIDTH; k++)
        for (int j = 0; j < CHUNK_HEIGHT; j++)
            for (int i = 0; i < CHUNK_WIDTH; i++)
                for (int face = 0; face < 6; face++) // 6 faces for each cube
                    for (int v = 0; v < 6; v++)
                    { // 6 vertices for each face
                        if (blocks.blockAt(i, j, k) == AIR)
                            continue;
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
                                static_cast<GLfloat>(sideLookup(blocks.blockAt(i, j, k), dir[face]))
                        );
                    }

    // note that new mesh was constructed based on changes
    // pretty sure this is an lvalue so this works
    registry.get<ChunkComponent>(chunk).hasChanged = false;
    registry.get<MeshComponent>(chunk).mustUpdateBuffer = true;
}

void ChunkMeshingSystem::greedyMesh(entt::entity &chunk, entt::registry &registry)
{
    // // wireframes for debugging
//     glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    // retrieve refs to block data & vertex storage
    ChunkComponent& blocks = registry.get<ChunkComponent>(chunk);
    std::vector<texArrayVertex>& vertices = registry.get<MeshComponent>(chunk).chunkVertices;
    glm::vec3& pos = registry.get<PositionComponent>(chunk).pos;

    vertices.clear(); // delete old vertex data

    Direction bFaceDirs[3] = {EAST, UP, NORTH}; // x, y, z indexing to support dim, u, v indexing
    Direction fFaceDirs[3] = {WEST, DOWN, SOUTH};
    // x, y, z indexing to support dim, u, v indexing: gives max index in chunk array
    int chunkDimSize[3] = {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH};

    // ---------------------- GREEDY MESHING ALGORITHM ----------------------

    // sweep over each dimension (constructs both faces per dim)
    // dim = dimension perpendicular to mask face
    for (int dim = 0; dim < 3; dim++)
    {
        Direction bDir = bFaceDirs[dim];
        Direction fDir = fFaceDirs[dim];
        // track dimensions of mask plane
        int u = (dim + 1) % 3;
        int v = (dim + 2) % 3;
        // dVec = unit vector perpendicular to mask
        int dVec[3] = {0, 0, 0};
        dVec[dim] = 1;
        // tracks position (relative to local voxel coordinates where start = (0,0,0) & max=(CHUNK_WIDTH-1, ...)
        int curVox[3];

        // tracks face type in UV plane that dim passes through
        BlockType mask[chunkDimSize[u] * chunkDimSize[v]]; // must explicitly define each entry, AIR!=default

        for (curVox[dim] = -1; curVox[dim] < chunkDimSize[dim]; ) // depth=N has N+1 faces
        {
            // ---------------------- COMPUTE MASK ----------------------
            for (curVox[v] = 0; curVox[v] < chunkDimSize[v]; curVox[v]++)
                for (curVox[u] = 0; curVox[u] < chunkDimSize[u]; curVox[u]++)
                {
                    // voxels behind + in front of face of interest
                    BlockType bFace = (curVox[dim] >= 0) ?
                                      sideLookup(blocks.blockAt(curVox[0], curVox[1], curVox[2]), bDir) : AIR;
                    BlockType fFace = (curVox[dim] < chunkDimSize[dim] - 1) ?
                            sideLookup(blocks.blockAt(curVox[0] + dVec[0],
                                                      curVox[1] + dVec[1],
                                                      curVox[2] + dVec[2]), fDir)
                                                                      : AIR;

                    // only draw face if EXACTLY one side is AIR
                    mask[curVox[u] + curVox[v] * chunkDimSize[u]] = ((bFace != AIR) != (fFace != AIR)) ? ((bFace != AIR) ? bFace : fFace) : AIR;
                }

            // starts at -1 for first face, which is truly blockAt 0 relative to chunk --> inc reflects face position
            curVox[dim]++;

            // ---------------------- GENERATE MESH FOR MASK ----------------------
            // iterate across mask, grouping together equal adjacent faces
            for (int j = 0; j < chunkDimSize[v]; j++)
            {
                for (int i = 0; i < chunkDimSize[u];)
                {
                    BlockType curFace = mask[i + j * chunkDimSize[u]];
                    if (curFace == AIR) {
                        i++;
                        continue; // ignore blank faces
                    }

                    // find maximum width of identically drawn faces
                    int width = 1; // absolute length
                    while (i + width < chunkDimSize[u] && mask[i + width + j * chunkDimSize[u]] == curFace)
                    {
                        width++;
                    }
                    // find maximum height (given width) of identically drawn faces
                    int height = 1; // absolute length
                    while (j + height < chunkDimSize[v])
                    {
                        // move across width-direction, checking all blocks same as curFace
                        int wIncrement = 0;
                        while (wIncrement < width && mask[i + wIncrement + (j+height) * chunkDimSize[u]] == curFace)
                        { wIncrement++; }
                        if (wIncrement == width) // entire height column matches curFace, can append entire column
                            height++;
                        else // can only increment height if full width-length column is of same texture
                            break;
                    }

                    // store quad vertices for rendering (ultimately 2 triangles)
                    curVox[u] = i; // curVox[dim] already accurate, [u] and [v] reset on next mask creation
                    curVox[v] = j;
                    // dU and dV are vectors in u/v directions of width/height lengths, curVox+dU+dV = end of quad
                    int dU[3] = {0, 0, 0};
                    dU[u] = width;
                    int dV[3] = {0, 0, 0};
                    dV[v] = height;

                    glm::vec3 vStart, vU, vV, vEnd; // vU stretches in u/width dir, vV stretches in v/height dir
                    // note: must offset by chunk location (pos = vec3, absolute chunk location)
                    // curVox (absolute location, not within-chunk location)
                    vStart[0] = pos[0] + curVox[0]; vStart[1] = pos[1] + curVox[1]; vStart[2] = pos[2] + curVox[2];
                    // vU = curVox corner offset by width (curVox + dU)
                    vU[0] = pos[0] + curVox[0] + dU[0]; vU[1] = pos[1] + curVox[1] + dU[1]; vU[2] = pos[2] + curVox[2] + dU[2];
                    // vV = curVox corner offset by height (curVox + dV)
                    vV[0] = pos[0] + curVox[0] + dV[0]; vV[1] = pos[1] + curVox[1] + dV[1]; vV[2] = pos[2] + curVox[2] + dV[2];
                    // vEnd = opposite curVox start corner (curVox + dU + dV)
                    vEnd[0] = pos[0] + curVox[0] + dU[0] + dV[0]; vEnd[1] = pos[1] + curVox[1] + dU[1] + dV[1];
                    vEnd[2] = pos[2] + curVox[2] + dU[2] + dV[2];

                    // TODO: include normals in .equals() to support 6 unique faces instead of 3 (also lighting values)
                    Direction dirs[3] = {WEST, DOWN, SOUTH};
                    appendQuad(vStart, vU, vV, vEnd, curFace, width, height, dirs[dim], vertices);

                    // clear mask for subsequent passes (prevents drawing same face again)
                    for (int h = 0; h < height; h++)
                        for (int w = 0; w < width; w++)
                            mask[i + w + (j + h) * chunkDimSize[u]] = AIR;

                    i += width;
                }
            }
        }
    }

    // note that new mesh was constructed based on changes
    // pretty sure this is an lvalue so this works
    registry.get<ChunkComponent>(chunk).hasChanged = false;
    registry.get<MeshComponent>(chunk).mustUpdateBuffer = true;
}

// appends a face to the texArrayVertex vector (two triangles/6 vertices, "quad" for short)
void ChunkMeshingSystem::appendQuad(glm::vec3 vStart, glm::vec3 vWidth, glm::vec3 vHeight, glm::vec3 vEnd,
                              BlockType block, int width, int height,
                              Direction dir, std::vector<texArrayVertex>& vertices) {

    // draw out the UV coords on paper to ensure sides drawn same, top/bottom face same way
    // make a new member variable array, indexed by Direction, of uvCoords arrays
    // might need to std::swap width/height for WEST/EAST... similar for DOWN/UP?
    // ^ NEED MORE COMPLICATED TESTS AFTER

    // WEST EAST messed up
    if (dir == WEST || dir == EAST)
    {
        vertices.emplace_back(vStart.x, vStart.y, vStart.z, uvCoords[1]*height, uvCoords[0]*width, sideLookup(block, dir));
        vertices.emplace_back(vWidth.x, vWidth.y, vWidth.z, uvCoords[3]*height, uvCoords[2]*width, sideLookup(block, dir));
        vertices.emplace_back(vHeight.x, vHeight.y, vHeight.z, uvCoords[5]*height, uvCoords[4]*width, sideLookup(block, dir));
        vertices.emplace_back(vHeight.x, vHeight.y, vHeight.z, uvCoords[7]*height, uvCoords[6]*width, sideLookup(block, dir));
        vertices.emplace_back(vEnd.x, vEnd.y, vEnd.z, uvCoords[9]*height, uvCoords[8]*width, sideLookup(block, dir));
        vertices.emplace_back(vWidth.x, vWidth.y, vWidth.z, uvCoords[11]*height, uvCoords[10]*width, sideLookup(block, dir));
    } else
    {
        vertices.emplace_back(vStart.x, vStart.y, vStart.z, uvCoords[0]*width, uvCoords[1]*height, sideLookup(block, dir));
        vertices.emplace_back(vWidth.x, vWidth.y, vWidth.z, uvCoords[2]*width, uvCoords[3]*height, sideLookup(block, dir));
        vertices.emplace_back(vHeight.x, vHeight.y, vHeight.z, uvCoords[4]*width, uvCoords[5]*height, sideLookup(block, dir));
        vertices.emplace_back(vHeight.x, vHeight.y, vHeight.z, uvCoords[6]*width, uvCoords[7]*height, sideLookup(block, dir));
        vertices.emplace_back(vEnd.x, vEnd.y, vEnd.z, uvCoords[8]*width, uvCoords[9]*height, sideLookup(block, dir));
        vertices.emplace_back(vWidth.x, vWidth.y, vWidth.z, uvCoords[10]*width, uvCoords[11]*height, sideLookup(block, dir));
    }

}

