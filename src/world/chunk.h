#ifndef CHUNK_H
#define CHUNK_H

#define RENDER_DISTANCE 5
#define NUM_VISIBLE_CHUNKS ((2 * RENDER_DISTANCE + 1) * (2 * RENDER_DISTANCE + 1))

#include "mesh/chunk_mesh.h"

typedef struct Chunk {
    int dirty;
    MeshHandle handle;
} Chunk;

typedef struct ChunkMap {
    
} ChunkMap;

Chunk createChunk(int dirty, int handle);
void destroyChunk(Chunk **chunk);

#endif