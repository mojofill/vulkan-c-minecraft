#include "chunk.h"
#include <stdlib.h>

void destroyChunk(Chunk **chunk) {
    // gonna need to add some stuff here
}

void chunkPutBlock(Chunk *chunk, int local_x, int local_y, int local_z, int type) {
    chunk->blocks[chunk_mesh_xyz_to_block_index(local_x, local_y, local_z)] = type;
}
