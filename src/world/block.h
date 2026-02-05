#ifndef BLOCK_H
#define BLOCK_H

#include "cglm/cglm.h"

typedef enum BlockType {
    AIR,
    GRASS, // has special properties, because side is different texture
    SMOOTH_STONE,
    DIRT,
    OAK_PLANK,
    OAK_LOG_SIDE,
    COBBLE_STONE,
    WHITE_WOOL,
    SAND,
    GRAVEL
} BlockType;

void blockTypeToAtlasCoord(BlockType type, int *u, int *v);

#endif