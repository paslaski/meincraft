#pragma once

enum BlockType // : uint8_t
{
    // must correspond to texture atlas position
    // NOTE: 0-indexed positions
    AIR,
    STONE = 15*16 + 1,
    DIRT = 15*16 + 2,
    GRASS,
    GRASS_SIDE = 15*16 + 3,
    GRASS_TOP = 3*16 + 12,
    COBBLESTONE = 14*16 + 0,
    SAND = 14*16 + 2,
    WOOL_YELLOW = 5*16 + 2
};

// current dim scheme (face = 0 through 5)
// SOUTH, WEST, DOWN, NORTH, EAST, UP
enum Direction
{
    SOUTH = 0,
    WEST = 1,
    DOWN = 2,
    NORTH = 3,
    EAST = 4,
    UP = 5
};

BlockType sideLookup(BlockType block, Direction direction);

//struct Block {
//    BlockType block = BlockType::DIRT; // = Block::BlockType::AIR;
//    // lighting
//};