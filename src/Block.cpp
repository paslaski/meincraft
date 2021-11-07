
#include <array>
#include <unordered_map>
#include "Block.h"

std::unordered_map<BlockType, std::array<BlockType, 6>> sideEnumerator = {
        // current dim scheme (face = 0 through 5)
        // SOUTH, WEST, DOWN, NORTH, EAST, UP
        {GRASS, std::array<BlockType,6>{GRASS_SIDE, GRASS_SIDE, DIRT, GRASS_SIDE, GRASS_SIDE, GRASS_TOP}}
};

BlockType sideLookup(BlockType block, Direction direction)
{
    // use std::unordered_map<BlockType, BlockType> with default returning itself when no match found
    // default = not side dependent, return self
    if (sideEnumerator.contains(block)) {
        std::array<BlockType, 6> blockArr = sideEnumerator[block];
        return blockArr.at((int) direction); // direction enum aligns with indices
    }
    else
        return block;
};