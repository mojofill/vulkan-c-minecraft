#include "world.h"

void createWorld(World *world) {
    Camera cam = {
        .pos   = {0.0f, 2.0f, 10.0f},
        .pitch = -GLM_PI_4f,        // looking level
        .yaw   = 0    // look towards -Y axis
    };

    // Build initial dir from yaw+pitch
    cam.dir[0] = cosf(cam.pitch) * cosf(cam.yaw);
    cam.dir[1] = cosf(cam.pitch) * sinf(cam.yaw);
    cam.dir[2] = sinf(cam.pitch);
    glm_normalize(cam.dir);

    world->cam = cam;

    // allocate space for chunk map and chunk pool
    ChunkMap map = {0};
    ChunkPool pool = {0};

    chunk_map_init(&map, MAX_LOADED_CHUNKS);

    world->chunkMap = map;
    world->chunkPool = pool;
}

void destroyWorld(World *world) {
    chunk_map_destroy(&(world->chunkMap));
    chunk_pool_destroy(&(world->chunkPool));
}
