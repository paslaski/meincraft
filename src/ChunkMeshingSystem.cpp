
#include "ChunkMeshingSystem.h"
#include <iostream>

ChunkMeshingSystem::ChunkMeshingSystem()
{}

ChunkMeshingSystem::~ChunkMeshingSystem()
{}

void ChunkMeshingSystem::update(entt::registry& registry)
{
    // view all chunks (data stored in BlockComponents)
    auto chunkView = registry.view<ChunkComponent>();
    for (const auto& chunk : chunkView)
    {
        // only have to update mesh if blocks have changed
        if (registry.get<ChunkComponent>(chunk).hasChanged())
            constructMesh(chunk, registry);
//            greedyMesh(chunk, registry);
    }
}

void ChunkMeshingSystem::constructMesh(entt::entity chunk, entt::registry& registry)
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
                        if (blocks.blockAt(i, j, k)->typeOf() == AIR)
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
                                static_cast<GLfloat>(blocks.blockAt(i, j, k)->sideAtDir(dir[face])),
                                0xFF
                        );
                    }

    // note that new mesh was constructed based on changes
    // pretty sure this is an lvalue so this works
    registry.get<ChunkComponent>(chunk).markChangesResolved();
    registry.get<MeshComponent>(chunk).mustUpdateBuffer = true;
}

void ChunkMeshingSystem::greedyMesh(entt::entity chunk, entt::registry& registry)
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
        // track dimensions of blockMask plane
        int u = (dim + 1) % 3;
        int v = (dim + 2) % 3;
        // dVec = unit vector perpendicular to blockMask
        int dVec[3] = {0, 0, 0};
        dVec[dim] = 1;
        // tracks position (relative to local voxel coordinates where start = (0,0,0) & max=(CHUNK_WIDTH-1, ...)
        std::vector<int> curVox(3, 0);

        // dirs[dim][bFace == AIR] gives direction of face from voxel
        std::vector<std::vector<Direction>> dirs {{WEST, EAST}, {DOWN, UP}, {SOUTH, NORTH}};

        // tracks face type in UV plane that dim passes through
        BlockType blockMask[chunkDimSize[u] * chunkDimSize[v]]; // must explicitly define each entry, AIR!=default

        for (curVox[dim] = -1; curVox[dim] < chunkDimSize[dim]; ) // depth=N has N+1 faces
        {
            std::vector<uint8_t> lightMask(chunkDimSize[u] * chunkDimSize[v], 0);
            std::vector<int> dirMask(chunkDimSize[u] * chunkDimSize[v], -1); // for invalid enum default value

            // ---------------------- COMPUTE MASK ----------------------
            for (curVox[v] = 0; curVox[v] < chunkDimSize[v]; curVox[v]++)
                for (curVox[u] = 0; curVox[u] < chunkDimSize[u]; curVox[u]++)
                {
                    // voxels behind + in front of face of interest
                    BlockType bFace = (curVox[dim] >= 0) ?
                                      blocks.blockAt(curVox[0], curVox[1], curVox[2])->sideAtDir(bDir) : AIR;
                    BlockType fFace = (curVox[dim] < chunkDimSize[dim] - 1) ?
                                      blocks.blockAt(curVox[0] + dVec[0],
                                                     curVox[1] + dVec[1],
                                                     curVox[2] + dVec[2])->sideAtDir(fDir) : AIR;

                    // only draw face if EXACTLY one side is AIR
                    blockMask[curVox[u] + curVox[v] * chunkDimSize[u]] = ((bFace == AIR) != (fFace == AIR)) ?
                                                                                ((bFace != AIR) ? bFace : fFace) : AIR;
                    // identify direction of face from non-AIR block
                    dirMask[curVox[u] + curVox[v] * chunkDimSize[u]] = dirs[dim][(bFace == AIR)];

                    // track light level of drawn face (light at AIR block)
                    // must supply x, y, z coordinates of non-AIR block
                    int x, y, z;
                    if (bFace == AIR) // get light level adjacent AIR block
                        lightMask[curVox[u] + curVox[v] * chunkDimSize[u]]
                                = getLightLevelAt(registry, chunk, blocks, curVox[0], curVox[1], curVox[2]);
                    else // fFace, one step in dVec (across face), is AIR
                        lightMask[curVox[u] + curVox[v] * chunkDimSize[u]]
                                = getLightLevelAt(registry, chunk, blocks, curVox[0] + dVec[0],
                                                  curVox[1] + dVec[1], curVox[2] + dVec[2]);

//                    if (bFace == AIR) // non-AIR block is dVec forward at fFace position
//                        x = curVox[0] + dVec[0], y = curVox[1] + dVec[1], z = curVox[2] + dVec[2];
//                    else // non-AIR block is at curVox, no need to step in dVec direction
//                        x = curVox[0], y = curVox[1], z = curVox[2];
//                    lightMask[curVox[u] + curVox[v] * chunkDimSize[u]]
//                        = getLightLevel(m_Registry, chunk, blocks, x, y, z, dirs[dim][(bFace == AIR)]);
                }

            // starts at -1 for first face, which is truly blockAt 0 relative to chunk --> inc reflects face position
            curVox[dim]++;

            // ---------------------- GENERATE MESH FOR MASK ----------------------
            // iterate across blockMask, grouping together equal adjacent faces
            for (int j = 0; j < chunkDimSize[v]; j++)
            {
                for (int i = 0; i < chunkDimSize[u];)
                {
                    BlockType curFace = blockMask[i + j * chunkDimSize[u]];
                    uint8_t curLightLevel = lightMask[i + j * chunkDimSize[u]];
                    if (curFace == AIR) {
                        i++;
                        continue; // ignore blank faces
                    }

                    // find maximum width of identically drawn faces (block face and light level)
                    int width = 1; // absolute length
                    while (i + width < chunkDimSize[u] && blockMask[i + width + j * chunkDimSize[u]] == curFace
                                                       && lightMask[i + width + j * chunkDimSize[u]] == curLightLevel)
                    {
                        width++;
                    }
                    // find maximum height (given width) of identically drawn faces
                    int height = 1; // absolute length
                    while (j + height < chunkDimSize[v])
                    {
                        // move across width-direction, checking all faces are identical (face type, light)
                        int wIncrement = 0;
                        while (wIncrement < width && blockMask[i + wIncrement + (j + height) * chunkDimSize[u]] == curFace
                                                  && lightMask[i + wIncrement + (j + height) * chunkDimSize[u]] == curLightLevel)
                        { wIncrement++; }
                        if (wIncrement == width) // entire height column matches curFace, can append entire column
                            height++;
                        else // can only increment height if full width-length column is of same texture
                            break;
                    }

                    // store quad vertices for rendering (ultimately 2 triangles)
                    curVox[u] = i; // curVox[dim] already accurate, [u] and [v] reset on next blockMask creation
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

                    appendQuad(vStart, vU, vV, vEnd, curFace, width, height, dirs[dim][0], vertices, curLightLevel);

                    // clear masks for subsequent passes (prevents drawing same face again)
                    for (int h = 0; h < height; h++)
                        for (int w = 0; w < width; w++)
                            blockMask[i + w + (j + h) * chunkDimSize[u]] = AIR,
                            lightMask[i + w + (j + h) * chunkDimSize[u]] = 0,
                            dirMask[i + w + (j + h) * chunkDimSize[u]] = -1;

                    i += width;
                }
            }
        }
    }

    // note that new mesh was constructed based on changes
    // pretty sure this is an lvalue so this works
    registry.get<ChunkComponent>(chunk).markChangesResolved();
    registry.get<MeshComponent>(chunk).mustUpdateBuffer = true;
}

// appends a face to the texArrayVertex vector (two triangles/6 vertices, "quad" for short)
void ChunkMeshingSystem::appendQuad(glm::vec3 vStart, glm::vec3 vWidth, glm::vec3 vHeight, glm::vec3 vEnd,
                              BlockType block, int width, int height,
                              Direction dir, std::vector<texArrayVertex>& vertices,
                              uint8_t lightLevel)
                              {
//                                  (glm::vec3 vStart, glm::vec3 vWidth, glm::vec3 vHeight, glm::vec3 vEnd,
//                                          BlockType block, int width, int height,
//                                          Direction dir, std::vector<texArrayVertex>& vertices,
//                                          std::vector<int>& curVox,
//                                          entt::m_Registry& m_Registry, ChunkComponent& chunkComp)
    BlockType blockSide = sideLookup(block, dir);
//    GLubyte lightLevel;

//    if (dir == WEST && curVox[0] == 0) // x = -1 in current chunk refers to x = CW-1 in western neighbor
//        if (chunkComp.neighborEntities[WEST] != entt::null)
//            lightLevel = m_Registry.get<ChunkComponent>(chunkComp.neighborEntities[WEST]).lightAt(CHUNK_WIDTH-1, curVox[1], curVox[2]);
//        else
//            lightLevel = 0xFF;
//    else if (dir == EAST && curVox[0] == (CHUNK_WIDTH - 1)) // x = CW is x = 0 in eastern neighbor
//        if (chunkComp.neighborEntities[EAST] != entt::null)
//            lightLevel = m_Registry.get<ChunkComponent>(chunkComp.neighborEntities[EAST]).lightAt(0, curVox[1], curVox[2]);
//        else
//            lightLevel = 0xFF;
//    else if (dir == SOUTH && curVox[2] == 0) // z = -1 is z = CW-1 in southern neighbor
//        if (chunkComp.neighborEntities[SOUTH] != entt::null)
//            lightLevel = m_Registry.get<ChunkComponent>(chunkComp.neighborEntities[SOUTH]).lightAt(curVox[0], curVox[1], CHUNK_WIDTH-1);
//        else
//            lightLevel = 0xFF;
//    else if (dir == NORTH && curVox[2] == (CHUNK_WIDTH - 1)) // z = CW is z = 0 in northern neighbor
//        if (chunkComp.neighborEntities[NORTH] != entt::null)
//            lightLevel = m_Registry.get<ChunkComponent>(chunkComp.neighborEntities[NORTH]).lightAt(curVox[0], curVox[1], 0);
//        else
//            lightLevel = 0xFF;
//    else if (dir == DOWN && curVox[1] == 0)
//        lightLevel = 0xFF; // lets just give it direct sunlight? player couldn't ever view
//    else if (dir == UP && curVox[1] == (CHUNK_HEIGHT - 1))
//        lightLevel = 0xFF; // always direct sunlight
//    else // delta?ByDir[dir] and bounds checking
//        lightLevel = chunkComp.lightAt(curVox[0], curVox[1], curVox[2]);

    if (dir == WEST || dir == EAST)
    {
        vertices.emplace_back(vStart.x, vStart.y, vStart.z, uvCoords[1]*height, uvCoords[0]*width, blockSide, lightLevel);
        vertices.emplace_back(vWidth.x, vWidth.y, vWidth.z, uvCoords[3]*height, uvCoords[2]*width, blockSide, lightLevel);
        vertices.emplace_back(vHeight.x, vHeight.y, vHeight.z, uvCoords[5]*height, uvCoords[4]*width, blockSide, lightLevel);
        vertices.emplace_back(vHeight.x, vHeight.y, vHeight.z, uvCoords[7]*height, uvCoords[6]*width, blockSide, lightLevel);
        vertices.emplace_back(vEnd.x, vEnd.y, vEnd.z, uvCoords[9]*height, uvCoords[8]*width, blockSide, lightLevel);
        vertices.emplace_back(vWidth.x, vWidth.y, vWidth.z, uvCoords[11]*height, uvCoords[10]*width, blockSide, lightLevel);
    } else
    {
        vertices.emplace_back(vStart.x, vStart.y, vStart.z, uvCoords[0]*width, uvCoords[1]*height, blockSide, lightLevel);
        vertices.emplace_back(vWidth.x, vWidth.y, vWidth.z, uvCoords[2]*width, uvCoords[3]*height, blockSide, lightLevel);
        vertices.emplace_back(vHeight.x, vHeight.y, vHeight.z, uvCoords[4]*width, uvCoords[5]*height, blockSide, lightLevel);
        vertices.emplace_back(vHeight.x, vHeight.y, vHeight.z, uvCoords[6]*width, uvCoords[7]*height, blockSide, lightLevel);
        vertices.emplace_back(vEnd.x, vEnd.y, vEnd.z, uvCoords[8]*width, uvCoords[9]*height, blockSide, lightLevel);
        vertices.emplace_back(vWidth.x, vWidth.y, vWidth.z, uvCoords[10]*width, uvCoords[11]*height, blockSide, lightLevel);
    }

}

uint8_t ChunkMeshingSystem::getLightLevelAt(entt::registry& registry, const entt::entity& e_Chunk,
                                          ChunkComponent& chunkComp, const int x, const int y,
                                          const int z) {
    // x, y, z is the coordinate of the voxel with the face being inspected
    GLubyte lightLevel;

    if (x == -1) // x = -1 in current chunk refers to x = CW-1 in western neighbor
        if (chunkComp.neighborEntities[WEST] != entt::null)
            lightLevel = registry.get<ChunkComponent>(chunkComp.neighborEntities[WEST]).lightAt(CHUNK_WIDTH - 1, y, z);
        else
            lightLevel = 0xFF;
    else if (x == CHUNK_WIDTH) // x = CW is x = 0 in eastern neighbor
        if (chunkComp.neighborEntities[EAST] != entt::null)
            lightLevel = registry.get<ChunkComponent>(chunkComp.neighborEntities[EAST]).lightAt(0, y, z);
        else
            lightLevel = 0xFF;
    else if (y == -1) // let's just give it direct sunlight at bottom, no one will see for now
        lightLevel = 0xFF;
    else if (y == CHUNK_HEIGHT) // always direct sunlight
        lightLevel = 0xFF;
    else if (z == -1) // z = -1 is z = CW-1 in southern neighbor
        if (chunkComp.neighborEntities[SOUTH] != entt::null)
            lightLevel = registry.get<ChunkComponent>(chunkComp.neighborEntities[SOUTH]).lightAt(x, y, CHUNK_WIDTH - 1);
        else
            lightLevel = 0xFF;
    else if (z == CHUNK_WIDTH) // z = CW is z = 0 in northern neighbor
        if (chunkComp.neighborEntities[NORTH] != entt::null)
            lightLevel = registry.get<ChunkComponent>(chunkComp.neighborEntities[NORTH]).lightAt(x, y, 0);
        else
            lightLevel = 0xFF;
    else
        lightLevel = chunkComp.lightAt(x, y, z);

    return lightLevel;
}