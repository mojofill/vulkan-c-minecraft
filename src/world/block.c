#include "block.h"

void blockTypeToAtlasCoord(BlockType type, Direction dir, int *u, int *v) {
    *u = 0;
    *v = 0;
    switch (type) {
        case GRASS:
            if (dir != UP) {
                *u = 1;
            }
            break;
        case SMOOTH_STONE:
            *u = 2;
            break;
        case DIRT:
            *u = 3;
            break;
        case OAK_PLANK:
            *u = 4;
            break;
        case OAK_LOG_SIDE:
            *u = 5;
            break;
        case COBBLE_STONE:
            *u = 6;
            break;
        case WHITE_WOOL:
            *u = 7;
            break;
        case SAND:
            *u = 8;
            break;
        case GRAVEL:
            *u = 9;
            break;
        case SPRUCE_LOG_SIDE:
            // if (dir == UP) *u = 10;
            *u = 10;
            break;
        case SNOW:
            *u = 11;
            break;
        case WATER: // technically its blue wool lol
            *u = 12;
            break;
        case OAK_LEAVES:
            *u = 13;
            break;
    }
}
