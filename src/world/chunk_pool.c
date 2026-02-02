#include "chunk_pool.h"

// -- WARNING: I SHOULD NEVER CALL THIS MYSELF. ONLY CALL chunkCreate
ChunkHandle chunk_alloc(ChunkPool *pool) {
    for (uint32_t i = 0; i < MAX_LOADED_CHUNKS; i++) {
        if (!pool->used[i]) {
            pool->used[i] = 1;
            pool->chunks[i].chunkHandle = i;
            pool->chunks[i].dirty = 1;
            return i;
        }
    }
    return CHUNK_HANDLE_INVALID;
}

void chunk_free(ChunkPool *pool, ChunkHandle handle) {
    if (handle == CHUNK_HANDLE_INVALID) return;
    pool->used[handle] = 0;
    // free mesh buffers here
    // free block data if heap allocated
    Chunk *chunk = &(pool->chunks[handle]);
    free(chunk->blocks);
}

// adds chunk to pool, returns unique handle
ChunkHandle createChunk(ChunkPool *pool, ivec2 pos) {
    Chunk chunk = {0};
    glm_ivec2_copy(pos, chunk.pos);
    chunk.dirty = 1; // always dirty
    chunk.chunkHandle = chunk_alloc(pool);

    chunk.blocks = malloc(MAX_BLOCKS_PER_CHUNK * sizeof(BlockType));

    // add block data
    for (int i = 0; i < MAX_BLOCKS_PER_CHUNK; i++) {
        // row major
        chunk.blocks[i] = SMOOTH_STONE;
    }

    pool->chunks[chunk.chunkHandle] = chunk;
    return chunk.chunkHandle;
}

void chunk_pool_destroy(ChunkPool *pool) {
    // todo
}
