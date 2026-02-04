#include "block.h"

void blockToUVCoord(BlockType type, ivec2 uv) {
    int x = 0;
    int y = 0;
    switch (type) {
        case AIR:
            x = 15;
            y = 15; // last block leave as transparent
            break;
        case SMOOTH_STONE:
            x = 2;
            y = 0;
            break;
        case COBBLE_STONE:
            break;
    }

    uv[0] = x;
    uv[1] = y;
}