#ifndef STREAMER_H
#define STREAMER_H

#include <stdlib.h>
#include "world/chunk.h"

typedef struct Streamer {
    Chunk **activeChunks;
    int size;
} Streamer;

Streamer createStreamer();
void destroyStreamer(Streamer streamer);
void bindChunk(Streamer *streamer, Chunk *chunk);

#endif