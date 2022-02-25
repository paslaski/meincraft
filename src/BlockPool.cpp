
#include "BlockPool.h"

std::array<const Block*, TOTAL_BLOCK_TYPES> BlockPool::blockPtrs;

BlockPool::BlockPool()
{
    // create an instance of each object & store pointer
    for (auto& [type, isTransparent, name] : activeBlockRegistry)
        BlockPool::blockPtrs[static_cast<int>(type)] = new Block(type, isTransparent, name);
}

BlockPool::~BlockPool() {
    for (auto& ptr : BlockPool::blockPtrs)
        delete ptr;
}

const BlockPool& BlockPool::getPoolInstance() {
    static BlockPool bp;
    return bp;
}

const Block* BlockPool::getBlockPtr(const BlockType type) const {
    return BlockPool::blockPtrs[static_cast<int>(type)];
}