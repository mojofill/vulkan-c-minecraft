#include "camera.h"

void camera_move(Camera *cam, vec3 dir, float dt) {
    vec3 scaledDir;
    glm_vec3_scale(dir, dt, scaledDir);
    glm_vec3_add(cam->pos, scaledDir, cam->pos);
}

void camera_process_inputs(Camera *cam, GLFWwindow *window)
{
    const float moveSpeed = 0.005f;
    const float rotSpeed  = 0.005f;

    // --- UPDATE ROTATION (yaw + pitch) ---
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cam->yaw -= rotSpeed;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cam->yaw += rotSpeed;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cam->pitch += rotSpeed;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cam->pitch -= rotSpeed;

    // Clamp pitch to (-89°, +89°)
    float limit = GLM_PI_2 - 0.001f;
    cam->pitch = glm_clamp(cam->pitch, -limit, limit);

    // --- REBUILD dir FROM yaw + pitch ---
    cam->dir[0] = cosf(cam->pitch) * cosf(cam->yaw);
    cam->dir[1] = cosf(cam->pitch) * sinf(cam->yaw);
    cam->dir[2] = sinf(cam->pitch);

    glm_normalize(cam->dir);

    // Build flat forward (for WASD move)
    vec3 flatDir = { cam->dir[0], cam->dir[1], 0.0f };
    glm_normalize(flatDir);

    // Right = cross(flatDir, up)
    vec3 up = {0, 0, 1};
    vec3 right;
    glm_vec3_cross(flatDir, up, right);
    glm_normalize(right);

    // --- MOVEMENT ---
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_move(cam, flatDir, moveSpeed);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 back;
        glm_vec3_scale(flatDir, -1.0f, back);
        camera_move(cam, back, moveSpeed);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 left;
        glm_vec3_scale(right, -1.0f, left);
        camera_move(cam, left, moveSpeed);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_move(cam, right, moveSpeed);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera_move(cam, up, moveSpeed);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
    {
        vec3 down = {0, 0, -1};
        camera_move(cam, down, moveSpeed);
    }
}
