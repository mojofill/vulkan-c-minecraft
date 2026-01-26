#include "renderer/renderer.h"
#include "mesh/mesh_pool.h"
#include "world/world.h"
#include "streamer/streamer.h"

void processInput(GLFWwindow *window, Camera *cam) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    camera_process_inputs(cam, window);
}

void updateCameraUniforms(vk_context *vko, uint32_t currentImage, Camera cam) {
    UniformBufferObject ubo = {0};
    glm_mat4_identity(ubo.model);
    // glm_translate(ubo.model, (vec3){0.0001f * (time_ms() - vko->start_time), 0.0f, 0.0f});
    // glm_rotate(ubo.model, GLM_PI_2 * 0.0001f * (time_ms() - vko->start_time), (vec3){0.0f, 0.0f, 1.0f}); // for now this does not matter

    vec3 forward;
    glm_vec3_add(cam.pos, cam.dir, forward);

    glm_lookat(cam.pos, forward, (vec3){0.0f, 0.0f, 1.0f}, ubo.view);

    float width = (float) vko->surfaceCapabilities.currentExtent.width;
    float height = (float) vko->surfaceCapabilities.currentExtent.height;
    glm_perspective(GLM_PI_4, width / height, 0.1f, 10.0f, ubo.proj);

    ubo.proj[1][1] *= -1; // flip image upside down bc vulkan and opengl y axes are flipped
    memcpy(vko->cameraUniformBufferMapped[currentImage], &ubo, sizeof(ubo));
}

void mainLoop(vk_context *vko, Streamer *streamer, World *world, MeshPool *pool) {
    uint32_t currentFrame = 0;
    while (!glfwWindowShouldClose(vko->window)) {
        glfwPollEvents();
        processInput(vko->window, &world->cam);
        updateCameraUniforms(vko, currentFrame, world->cam);
        // update chunk (cpu) data into streamer (only updates, doesnt do unnecessary work)
        // this is already uploaded with a simple test function, but later obviously should upload from a chunk_map
        // after streamer has cpu chunk data, streamer needs to be connected to mesh pool to reallocation

        syncStreamerWithMeshPool(streamer, pool);
        
        drawFrame(vko, &currentFrame, *pool);
    }

    vkDeviceWaitIdle(vko->device);
}

void cleanup(vk_context *vko, World world, Streamer streamer, MeshPool pool) {
    destroyMeshPool(pool);
    destroyWorld(world);
    destroyStreamer(streamer);
    cleanupRenderer(vko);
}

int main() {
    vk_context vko = {0};
    VertexBufferContext vbo = {0};
    vko.vbo = &vbo;

    World world = createWorld();

    MeshPool meshPool = {0};
    createMeshPool(&meshPool, NUM_VISIBLE_CHUNKS);

    Streamer streamer = createStreamer();

    // for testing purposes, will create a single chunk with meshHandle 0
    Chunk chunk = createChunk(1, 0);
    streamer.activeHandles[0] = chunk.meshHandle;

    // for now i will manually allocate and free mesh pool with chunk data

    mesh_alloc(&meshPool, 0);
    syncStreamerWithMeshPool(&streamer, &meshPool);

    // what i need to do right now: write out the system a way that uses handles to transport data

    init_renderer(&vko);
    mainLoop(&vko, &streamer, &world, &meshPool);
    cleanup(&vko, world, streamer, meshPool);
    
    return 0;
}