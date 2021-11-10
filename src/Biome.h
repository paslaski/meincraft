
#pragma once

#include "Block.h"

enum BiomeType {
    GrassBiome = 0,
    SandBiome
};

BlockType biomeTopBlockLookup(BiomeType biome);