#include "streamer.h"
#include <stdio.h>

Streamer createStreamer() {
    Streamer self = {0};
    self.size = NUM_VISIBLE_CHUNKS;
    self.activeHandles = malloc(sizeof(MeshHandle) * self.size);
    for (int i = 0; i < self.size; i++) {
        self.activeHandles[i] = MESH_HANDLE_INVALID;
    }
    return self;
}

// finds difference between streamer handles vs mesh pool handles
// this function only *adds* meshes. i have to manually free the chunks that are out of sight
void syncStreamerWithMeshPool(Streamer *streamer, MeshPool *pool) {
    // will do this later
}

void destroyStreamer(Streamer streamer) {
    free(streamer.activeHandles);
}
