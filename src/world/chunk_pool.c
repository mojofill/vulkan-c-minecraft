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

static inline void rotate2D(float x, float z, float angle, float *rx, float *rz) {
    float c = cosf(angle);
    float s = sinf(angle);
    *rx = c * x - s * z;
    *rz = s * x + c * z;
}

float fbm(int wx, int wz) {
    float value = -0.2f;
    float amplitude = 1.5f;
    float frequency = 0.025f;
    float maxAmp = 0;

    float x = (float)wx;
    float z = (float)wz;

    rotate2D(x, z, 0.7f, &x, &z);

    for (int i = 0; i < 4; i++) {
        value += amplitude * stb_perlin_noise3(
            x * frequency,
            0.0f,
            z * frequency,
            0, 0, 0
        );

        maxAmp += amplitude;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    return value / maxAmp; // now in [-1,1]
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
    chunk.num_blocks = 0;
    chunk.num_surface_blocks = 0;
    chunk.blocks = malloc(MAX_BLOCKS_PER_CHUNK * sizeof(BlockType));

    chunk_mesh_foreach(x, y, z) {        
        int wx = chunk.pos[0] * CHUNK_BLOCK_WIDTH + x;
        int wy = chunk.pos[1] * CHUNK_BLOCK_WIDTH + y;
        
        float height = heightFunc(wx, wy);

        // printf("height: %f\n", height);

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
        
        if (z >= height - 15) type = AIR; // air
        else {
            if (z > height - 35) type = AIR; // air 
            else { 
                // type = (int) (randf() * 10.0f) + 1; 
                if (z == 0) type = WATER;
                else if (z == 1) type = SAND;
                else if (z == 2) type = DIRT;
                else if (z <= 3) type = GRASS;
                else if (z <= 10) type = SMOOTH_STONE;
                else type = SNOW;
                
                chunk.num_blocks++; 
            }
            chunk.num_blocks++;
        }

        int i = chunk_mesh_xyz_to_block_index(x, y, z);
        chunk.blocks[i] = type;
    }

    // now see how many surface blocks are there
    for (int y = 0; y < CHUNK_BLOCK_WIDTH; y++) {
        for (int x = 0; x < CHUNK_BLOCK_WIDTH; x++) {
            for (int z = CHUNK_BLOCK_HEIGHT-1; z >= 0; z--) {
                int idx = chunk_mesh_xyz_to_block_index(x,y,z);
                BlockType type = chunk.blocks[idx];
                if (type != AIR) {
                    chunk.num_surface_blocks++;
                    break;
                }
                if (z == 0 && type == AIR) {
                    chunk.blocks[idx] = WATER;
                    chunk.num_surface_blocks++;
                    chunk.num_blocks++;
                }
            }
        }
    }

    pool->chunks[chunk.chunkHandle] = chunk;
    return chunk.chunkHandle;
}

void chunk_pool_destroy(ChunkPool *pool) {
    for (int i = 0; i < pool->count; i++) {
        chunk_free(pool, pool->chunks[i].chunkHandle);
    }
}
