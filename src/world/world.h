#ifndef WORLD_H
#define WORLD_H

#include "camera.h"
#include "chunk.h"

#define MAX_LOADED_CHUNKS UINT32_MAX

typedef struct World {
    Camera cam;
    Chunk *chunks;
    int chunkCount;
} World;

void createChunk(World *world, vec2 pos);
void createWorld(World *world);
void destroyWorld(World world);

#endif