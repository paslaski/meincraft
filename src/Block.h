#pragma once

#include <array>
#include <vector>
#include <list>
#include <string>

const int TOTAL_BLOCK_TYPES = 16*16 + 1; // 16 by 16 atlas + air block
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

const std::array<BlockType, 6> sidesByType(BlockType type); // look up all six sides of block

// make a pool that inits objects ez
class Block {
private:
    const BlockType type;
    const std::array<BlockType, 6> sides;
    const bool transparent;
    const std::string typeName;
public:
    Block(const BlockType type, const bool trans, const std::string name)
            : type(type), sides(sidesByType(type)), transparent(trans), typeName(name) {}
    Block() = delete;

    const bool isTransparent() const {
        return transparent;
    };
    const BlockType sideAtDir(Direction dir) const {
        return sides[static_cast<int>(dir)];
    }
    const BlockType typeOf() const {
        return type;
    }
};

struct blockData {
    blockData(const BlockType t, const bool ts, const std::string n)
    :   type(t), transparencyStatus(ts), name(n) {}
    const BlockType type;
    const bool transparencyStatus;
    const std::string name;
};

static std::list<const blockData> activeBlockRegistry {
        {AIR, true, "Air"},
        {STONE, false, "Stone"},
        {GRASS, false, "Grass"},
        {COBBLESTONE, false, "Cobblestone"},
        {SAND, false, "Sand"},
        {WOOL_YELLOW, false, "Yellow Wool"},
        {LADDER, false, "Ladder"},
};

// WEST = -1, EAST = +1
const std::vector<int> deltaXByDir {0, 0, -1, 1, 0, 0};
// SOUTH = -1, NORTH = +1
const std::vector<int> deltaZByDir {1, -1, 0, 0, 0, 0};
// DOWN = -1, UP = +1
const std::vector<int> deltaYByDir {0, 0, 0, 0, 1, -1};

BlockType sideLookup(BlockType block, Direction direction);

