#include "chunk_pool.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#include <stdlib.h>

float randf() {
    return rand() / (float) RAND_MAX;
}

// -- WARNING: I SHOULD NEVER CALL THIS MYSELF. ONLY CALL chunkCreate
ChunkHandle chunk_alloc(ChunkPool *pool) {
    for (uint32_t i = 0; i < MAX_LOADED_CHUNKS; i++) {
        if (!pool->used[i]) {
            pool->used[i] = 1;
            pool->chunks[i].chunkHandle = i;
            pool->chunks[i].dirty = 1;
            pool->count++;
            return i;
        }
    }

    return CHUNK_HANDLE_INVALID;
}

void chunk_free(ChunkPool *pool, ChunkHandle handle) {
    if (handle == CHUNK_HANDLE_INVALID) return;
    pool->used[handle] = 0;
    pool->count--;
    // free mesh buffers here
    // free block data if heap allocated
    Chunk *chunk = &(pool->chunks[handle]);
    free(chunk->blocks);
}

float fbm(int wx, int wz) {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.005f;

    for (int i = 0; i < 4; i++) {
        value += amplitude * stb_perlin_noise3(
            wx * frequency,
            0.0f,
            wz * frequency,
            0, 0, 0
        );

        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    return value;
}

float heightFunc(int wx, int wz) {
    float n = fbm(wx, wz);
    n = (n + 1.0f) * 0.5f;
    return 20.0f + n * 40.0f;
}

// adds chunk to pool, returns unique handle
ChunkHandle createChunk(ChunkPool *pool, ivec2 pos) {
    Chunk chunk = {0};
    glm_ivec2_copy(pos, chunk.pos);
    chunk.dirty = 1; // always dirty
    chunk.chunkHandle = chunk_alloc(pool);

    chunk.blocks = malloc(MAX_BLOCKS_PER_CHUNK * sizeof(BlockType));

    chunk_mesh_foreach(x, y, z) {        
        int wx = chunk.pos[0] * CHUNK_BLOCK_WIDTH + x;
        int wy = chunk.pos[1] * CHUNK_BLOCK_HEIGHT + y;
        
        float height = heightFunc(wx, wy);

        float biome = stb_perlin_noise3(
            wx * 0.001f,
            100.0f,
            wy * 0.001f,
            0, 0, 0
        );

        if (biome > 0.3f)
            height += 10; // mountains
        else if (biome < -0.3f)
            height -= 5; // plains

        BlockType type;
        
        if (z > height - 25) type = AIR; // air
        else {
            type = (int) (randf() * 10.0f) + 1;
            if (16 <= z && z <= 18) type = WHITE_WOOL;
            chunk.num_blocks++;
        }

        // type = SMOOTH_STONE;
        // chunk.num_blocks++;

        int i = chunk_mesh_xyz_to_block_index(x, y, z);
        chunk.blocks[i] = type;
    }

    pool->chunks[chunk.chunkHandle] = chunk;
    return chunk.chunkHandle;
}

void chunk_pool_destroy(ChunkPool *pool) {
    for (int i = 0; i < pool->count; i++) {
        chunk_free(pool, pool->chunks[i].chunkHandle);
    }
}
