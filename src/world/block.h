#ifndef BLOCK_H
#define BLOCK_H

#include "cglm/cglm.h"
#include "geometries/vertex.h"

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
    GRAVEL,
    SPRUCE_LOG_SIDE,
    SNOW,
    WATER,
    OAK_LEAVES
} BlockType;

void blockTypeToAtlasCoord(BlockType type, Direction dir, int *u, int *v);

#endif