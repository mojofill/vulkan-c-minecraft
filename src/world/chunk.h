#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <cglm/cglm.h>

#define RENDER_DISTANCE 5
#define NUM_VISIBLE_CHUNKS ((2 * RENDER_DISTANCE + 1) * (2 * RENDER_DISTANCE + 1))
// CHUNK_BLOCK_WIDTH number of blocks on width of chunk
#define CHUNK_BLOCK_WIDTH 5
#define MAX_BLOCKS_PER_CHUNK (CHUNK_BLOCK_WIDTH * CHUNK_BLOCK_WIDTH)

typedef uint32_t ChunkHandle;
#define CHUNK_HANDLE_INVALID UINT32_MAX

// chunk handle invariants
// 1. each chunk has unique handle
// 2. once chunk is instantiated, handle remains binded with it until termination

typedef struct Chunk {
    vec2 pos;
    int dirty;
    ChunkHandle chunkHandle;
} Chunk;

typedef struct ChunkMap {
    
} ChunkMap;

void destroyChunk(Chunk **chunk);

#endif