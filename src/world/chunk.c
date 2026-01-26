#include "chunk.h"
#include <stdlib.h>

Chunk createChunk(int dirty, MeshHandle handle) {
    Chunk chunk = {0};
    chunk.dirty = dirty;
    chunk.meshHandle = handle;
    return chunk;
}

void destroyChunk(Chunk **chunk) {
    // free(*chunk);
}
