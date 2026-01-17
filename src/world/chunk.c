#include "chunk.h"
#include <stdlib.h>

Chunk createChunk(int dirty, int handle) {
    Chunk chunk = {0};
    chunk.dirty = dirty;
    chunk.handle = handle;
    return chunk;
}

void destroyChunk(Chunk **chunk) {
    // free(*chunk);
}
