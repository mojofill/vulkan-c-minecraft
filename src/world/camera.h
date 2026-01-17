#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

typedef struct Camera {
    vec3 pos;
    vec3 dir;
    float pitch;
    float yaw;
} Camera;

void camera_move(Camera *cam, vec3 dir, float dt);
void camera_process_inputs(Camera *cam, GLFWwindow *window);

#endif