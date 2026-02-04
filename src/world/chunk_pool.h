#ifndef CHUNK_POOL_H
#define CHUNK_POOL_H

#include "chunk.h"

typedef struct {
    Chunk chunks[MAX_LOADED_CHUNKS]; // access chunk data through ChunkHandle
    uint8_t used[MAX_LOADED_CHUNKS];
    uint32_t count;
} ChunkPool;

ChunkHandle chunk_alloc(ChunkPool *pool);
void chunk_free(ChunkPool *pool, ChunkHandle handle);
ChunkHandle createChunk(ChunkPool *pool, ivec2 pos);
void chunk_pool_destroy(ChunkPool *pool);

#endif