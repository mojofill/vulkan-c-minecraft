#include "streamer.h"
#include <stdio.h>

void createStreamer(Streamer *streamer) {
    streamer->size = NUM_VISIBLE_CHUNKS;
    streamer->activeHandles = malloc(sizeof(ChunkHandle) * streamer->size);
    for (int i = 0; i < streamer->size; i++) {
        streamer->activeHandles[i] = CHUNK_HANDLE_INVALID;
    }
}

void destroyStreamer(Streamer streamer) {
    free(streamer.activeHandles);
}
