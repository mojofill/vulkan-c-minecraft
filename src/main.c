#include "renderer.h"
#include "game.h"

int main() {
    vk_context vko = {0};
    VertexBufferContext vbo = {0};
    vko.vbo = &vbo;

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

    vko.camera = &cam;

    Game game = {0};
    game.vko = &vko;
    game.cam = cam;

    run(&vko);
    return 0;
}