#ifndef CHUNK_MAP_H
#define CHUNK_MAP_H

#include "chunk.h"

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

void chunk_map_init(ChunkMap *map, uint32_t capacity);
void chunk_map_destroy(ChunkMap *map);
void chunk_map_remove(ChunkMap *map, int cx, int cy);
ChunkHandle chunk_map_get(ChunkMap *map, int cx, int cy);
void chunk_map_put(ChunkMap *map, int cx, int cy, ChunkHandle handle);

#endif