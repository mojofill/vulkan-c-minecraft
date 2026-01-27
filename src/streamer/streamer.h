#ifndef STREAMER_H
#define STREAMER_H

#include <stdlib.h>
#include "world/chunk.h"
#include "mesh/mesh_pool.h"

// streamer interaction with chunk handles
// * streamer.activeChunks = dynamic array of chunk handles that are to be drawn

typedef struct Streamer {
    ChunkHandle *activeHandles;
    int size;
} Streamer;

void createStreamer(Streamer *streamer);
void destroyStreamer(Streamer streamer);

#endif