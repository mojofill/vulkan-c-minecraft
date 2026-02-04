#ifndef BLOCK_H
#define BLOCK_H

#include "cglm/cglm.h"

typedef enum BlockType {
    AIR,
    SMOOTH_STONE,
    COBBLE_STONE,
    WOOD
} BlockType;

void blockToUVCoord(BlockType type, ivec2 uv);

#endif