#include "streamer.h"
#include <stdio.h>

Streamer createStreamer() {
    Streamer self = {0};
    self.size = NUM_VISIBLE_CHUNKS;
    self.activeChunks = malloc(sizeof(Chunk*) * self.size);
    for (int i = 0; i < self.size; i++) {
        self.activeChunks[i] = malloc(sizeof(Chunk));
    }
    return self;
}

// will probably not stay, but only used now for simple prototyping
void bindChunk(Streamer *streamer, Chunk *chunk) {
    streamer->activeChunks[chunk->handle] = chunk;
}

// if i recieve a seg fault here, most likely streamer chunks have not been allocated
void destroyStreamer(Streamer streamer) {
    for (int i = 0; i < streamer.size; i++) {
        destroyChunk(&(streamer.activeChunks[i]));
    }
    free(streamer.activeChunks);
}
