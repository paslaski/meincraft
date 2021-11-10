
#include "Biome.h"
#include <unordered_map>

std::unordered_map<BiomeType, BlockType> biomeBlockMap = {
        // current dim scheme (face = 0 through 5)
        // SOUTH, WEST, DOWN, NORTH, EAST, UP
        {GrassBiome, GRASS},
        {SandBiome, SAND}
};

BlockType biomeTopBlockLookup(BiomeType biome) {

    if (biomeBlockMap.contains(biome))
        return biomeBlockMap[biome];
    else
        return AIR;
}
