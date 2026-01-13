#ifndef GAME_H
#define GAME_H

#include "renderer.h" // circular includes right now
#include "camera.h"

/*
what should a game object have? it should have: vulkan help, camera, chunks (loaded, to_load, and to_unload), array of textures
*/

typedef struct Game {
    vk_context *vko;
    Camera cam;
} Game;

#endif