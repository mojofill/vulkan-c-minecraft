#ifndef WORLD_H
#define WORLD_H

#include "camera.h"
#include "chunk.h"

typedef struct World {
    Camera cam;
} World;

World createWorld();
void destroyWorld(World world);

#endif