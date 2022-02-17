#pragma once

#include <array>
#include <unordered_map>
#include "Block.h"

class BlockPool {
private:
    static std::array<const Block*, TOTAL_BLOCK_TYPES> blockPtrs; // pool of

    BlockPool() // private constructor, only one static object exists
    {
        // create an instance of each object & store pointer
        for (auto& [type, isTransparent, name] : activeBlockRegistry)
            blockPtrs[static_cast<int>(type)] = new Block(type, isTransparent, name);
    };
    ~BlockPool()
    {
        for (auto& ptr : blockPtrs)
            delete ptr;
    };
public:
    static BlockPool& getPoolInstance()
    {
        static BlockPool bp;
        return bp;
    }
    const Block* getBlockPtr(const BlockType type) const {
        return blockPtrs[static_cast<int>(type)];
    }

    BlockPool operator=(const BlockPool& bp) = delete; // singleton: disallow copies
    BlockPool(const BlockPool& bp) = delete;
};


