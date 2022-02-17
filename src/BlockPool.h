#pragma once

#include <array>
#include <unordered_map>
#include "Block.h"

class BlockPool {
private:
    static std::array<const Block*, TOTAL_BLOCK_TYPES> blockPtrs; // pool of pointers to single copy of blocks

    BlockPool(); // private constructor, only one static object exists
    ~BlockPool();
public:
    static BlockPool& getPoolInstance();
    const Block* getBlockPtr(const BlockType type) const;

    BlockPool operator=(const BlockPool& bp) = delete; // singleton: disallow copies
    BlockPool operator=(BlockPool&&) = delete;
    BlockPool(const BlockPool&) = delete;
    BlockPool(BlockPool&&) = delete;
};


