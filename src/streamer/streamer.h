#ifndef STREAMER_H
#define STREAMER_H

#include <stdlib.h>
#include "world/chunk.h"
#include "mesh/mesh_pool.h"

typedef struct Streamer {
    MeshHandle *activeHandles;
    int size;
} Streamer;

Streamer createStreamer();
void destroyStreamer(Streamer streamer);
void syncStreamerWithMeshPool(Streamer *streamer, MeshPool *pool);

#endif