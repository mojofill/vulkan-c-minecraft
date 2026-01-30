#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "cglm/include/cglm/cglm.h"

typedef uint32_t ChunkHandle;
#define CHUNK_HANDLE_INVALID UINT32_MAX

static inline uint64_t chunk_coord_key(int x, int y) {
    return ((uint64_t)(uint32_t)x << 32) | (uint32_t)y;
}

typedef struct ChunkMapEntry {
    uint64_t key;        // packed (x,z)
    ChunkHandle handle;
    int occupied;
} ChunkMapEntry;

typedef struct ChunkMap {
    ChunkMapEntry *entries;
    uint32_t capacity;
    uint32_t count;
} ChunkMap;

static inline uint32_t hash_u64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return (uint32_t)x;
}

void chunk_map_init(ChunkMap *map, uint32_t capacity) {
    map->capacity = capacity;
    map->count = 0;
    map->entries = malloc(sizeof(ChunkMapEntry) * capacity);
}
void chunk_map_destroy(ChunkMap *map) {
    free(map->entries);
}

// lowkey dont need this until later
void chunk_map_remove(ChunkMap *map, int cx, int cy) {
    uint64_t key = chunk_coord_key(cx, cy);
    uint32_t idx = hash_u64(key) % map->capacity;
}

ChunkHandle chunk_map_get(ChunkMap *map, int cx, int cy) {
    uint64_t key = chunk_coord_key(cx, cy);
    uint32_t idx = hash_u64(key) % map->capacity;

    for (;;) {
        ChunkMapEntry *e = &map->entries[idx];
        if (!e->occupied)
            return CHUNK_HANDLE_INVALID;
        if (e->key == key)
            return e->handle;
        idx = (idx + 1) % map->capacity;
    }
}

void chunk_map_put(ChunkMap *map, int cx, int cy, ChunkHandle handle) {
    uint64_t key = chunk_coord_key(cx, cy);
    uint32_t idx = hash_u64(key) % map->capacity;

    for (;;) {
        ChunkMapEntry *e = &map->entries[idx];
        if (!e->occupied) {
            e->occupied = 1;
            e->key = key;
            e->handle = handle;
            map->count++;
            return;
        }
        idx = (idx + 1) % map->capacity;
    }
}

typedef struct Chunk {
    vec2 pos;
    int dirty;
    ChunkHandle chunkHandle;
    // BlockType *blocks; // size = MAX_BLOCKS_PER_CHUNK
} Chunk;

#define MAX_CHUNKS 1024

// this would replace world.chunks
typedef struct {
    Chunk chunks[MAX_CHUNKS];
    uint8_t used[MAX_CHUNKS];
} ChunkPool;

ChunkHandle chunk_alloc(ChunkPool *pool) {
    for (uint32_t i = 0; i < MAX_CHUNKS; i++) {
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
}

int main() {
    ChunkMap map = {0};
    chunk_map_init(&map, 100);
    ChunkHandle handleIn = 0;
    chunk_map_put(&map, 1, 1, handleIn);
    ChunkHandle handleOut = chunk_map_get(&map, 1, 1);
    assert(handleIn == handleOut);
    printf("handleIn: %d\nhandleOut: %d\n", handleIn, handleOut);
    ChunkHandle handleInvalid = chunk_map_get(&map, 0, 0);
    assert(handleInvalid == CHUNK_HANDLE_INVALID);
    printf("grabbing not added chunk is CHUNK_HANDLE_INVALID\n");

    ChunkPool pool = {0};
    ChunkHandle handle1 = chunk_alloc(&pool);
    ChunkHandle handle2 = chunk_alloc(&pool);
    printf("handle1: %d, handle2: %d\nshould be 0, 1\n", handle1, handle2);

    chunk_free(&pool, handle1);   
}
