#pragma once


enum BlockType : uint8_t
{
    AIR = 0,
    DIRT,
    COBBLESTONE,
    GRASS,
};

struct Block {
    BlockType block = BlockType::DIRT; // = Block::BlockType::AIR;
    // lighting
};