
#include "entity.h"
#include "chunk.h"
#include "block.h"

void Chunk::createChunk(World& world)
{
    Entity e_chunk = world.create_entity("demo_chunk");
    e_chunk.add_component<PositionComponent>(glm::vec3( 0.0f,  0.0f,  0.0f));
    std::vector<BlockType> blocks(CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE, BlockType::DIRT);
    e_chunk.add_component<BlockComponent>(blocks);
}
