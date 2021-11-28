#pragma once

enum BlockType // : uint8_t
{
    // NOTE: AIR must be default value of BlockType
    AIR = 0,

    // must correspond to texture atlas position
    // NOTE: 1-indexed positions
    STONE = 15*16 + 1,
    DIRT = 15*16 + 2,
    GRASS,
    GRASS_SIDE = 15*16 + 3,
    GRASS_TOP = 3*16 + 12,
    COBBLESTONE = 14*16 + 0,
    SAND = 14*16 + 2,
    WOOL_YELLOW = 5*16 + 2,
    LADDER = 10*16 + 3
};

// current dim scheme (face = 0 through 5)
// SOUTH, WEST, DOWN, NORTH, EAST, UP
enum Direction
{
    NORTH = 0,
    SOUTH = 1,
    WEST = 2,
    EAST = 3,
    UP = 4,
    DOWN = 5
};

BlockType sideLookup(BlockType block, Direction direction);
