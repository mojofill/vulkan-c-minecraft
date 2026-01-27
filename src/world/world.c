#include "world.h"

void createChunk(World *world, vec2 pos) {
    Chunk chunk = {0};
    glm_vec2_copy(pos, chunk.pos);
    chunk.dirty = 1; // always dirty
    chunk.chunkHandle = world->chunkCount;

    world->chunks[chunk.chunkHandle] = chunk;
    world->chunkCount++;
}

void createWorld(World *world) {
    Camera cam = {
        .pos   = {0.0f, 2.0f, 2.0f},
        .pitch = -GLM_PI_4f,        // looking level
        .yaw   = -GLM_PI_2    // look towards -Y axis
    };

    // Build initial dir from yaw+pitch
    cam.dir[0] = cosf(cam.pitch) * cosf(cam.yaw);
    cam.dir[1] = cosf(cam.pitch) * sinf(cam.yaw);
    cam.dir[2] = sinf(cam.pitch);
    glm_normalize(cam.dir);

    world->cam = cam;

    world->chunks = malloc(sizeof(Chunk) * MAX_LOADED_CHUNKS);
    world->chunkCount = 0;
}

void destroyWorld(World world) {
    free(world.chunks);
}
