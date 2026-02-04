#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <cglm/cglm.h>
#include "block.h"

#define RENDER_DISTANCE 2
#define NUM_VISIBLE_CHUNKS ((2 * RENDER_DISTANCE + 1) * (2 * RENDER_DISTANCE + 1))
// CHUNK_BLOCK_WIDTH number of blocks on width of chunk
#define CHUNK_BLOCK_WIDTH 16
#define CHUNK_BLOCK_HEIGHT 32
#define MAX_BLOCKS_PER_CHUNK (CHUNK_BLOCK_WIDTH * CHUNK_BLOCK_WIDTH * CHUNK_BLOCK_HEIGHT)

#define MAX_LOADED_CHUNKS 4096

typedef uint32_t ChunkHandle;
#define CHUNK_HANDLE_INVALID UINT32_MAX

#define chunk_mesh_foreach(x, y, z) \
    for (int z = 0; z < CHUNK_BLOCK_HEIGHT; z++) \
        for (int y = 0; y < CHUNK_BLOCK_WIDTH; y++) \
            for (int x = 0; x < CHUNK_BLOCK_WIDTH; x++)

#define chunk_mesh_xyz_to_block_index(x, y, z) \
    (((z) * CHUNK_BLOCK_WIDTH * CHUNK_BLOCK_WIDTH) + \
     ((y) * CHUNK_BLOCK_WIDTH) + \
     (x))

// chunk handle invariants
// 1. each chunk has unique handle
// 2. once chunk is instantiated, handle remains binded with it until termination

// note: chunk has its own coordinate system
// ie chunk (0,0) is first chunk with CHUNK_BLOCK_WIDTH * CHUNK_BLOCK_WIDTH * CHUNK_BLOCK_HEIGHT blocks
typedef struct Chunk {
    ivec2 pos;
    int dirty;
    ChunkHandle chunkHandle;
    BlockType *blocks; // size = MAX_BLOCKS_PER_CHUNK
    uint32_t num_blocks;
} Chunk;

void destroyChunk(Chunk **chunk);

#endif