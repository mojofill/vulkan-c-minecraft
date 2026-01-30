#ifndef WORLD_H
#define WORLD_H

#include "camera.h"
#include "chunk.h"
#include "chunk_map.h"
#include "chunk_pool.h"

typedef struct World {
    Camera cam;
    ChunkMap chunkMap;
    ChunkPool chunkPool;
} World;

void createWorld(World *world);
void destroyWorld(World *world);

#endif