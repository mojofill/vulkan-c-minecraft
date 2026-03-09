#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define moveSpeed 0.5f
#define rotSpeed 0.025f
#define sensitivity 0.002f

typedef struct Camera {
    vec3 pos;
    vec3 dir;
    float pitch;
    float yaw;
    ivec2 chunkPos;
    double lastX;
    double lastY;
} Camera;

void camera_move(Camera *cam, vec3 dir, float dt);
void camera_process_inputs(Camera *cam, GLFWwindow *window);

#endif