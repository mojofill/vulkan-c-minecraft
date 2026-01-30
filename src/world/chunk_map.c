#include "chunk_map.h"

static inline uint64_t chunk_coord_key(int x, int y) {
    return ((uint64_t)(uint32_t)x << 32) | (uint32_t)y;
}

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
    fprintf(stderr, "not implemented yet\n");
    exit(1);
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
