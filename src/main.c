#include "renderer/renderer.h"
#include "mesh/mesh_pool.h"
#include "world/world.h"
#include "streamer/streamer.h"
#include "mach/mach_time.h"
#include <math.h>

static mach_timebase_info_data_t info;

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

    vec3 forward;
    glm_vec3_add(cam.pos, cam.dir, forward);

    float x = cam.pos[0];
    float y = cam.pos[1];
    float z = cam.pos[2];
    // printf("camera pos: %f, %f, %f\n", cam.pos[0], cam.pos[1], cam.pos[2]); // try to guess which chunk im in

    glm_lookat(cam.pos, forward, (vec3){0.0f, 0.0f, 1.0f}, ubo.view);

    float width = (float) vko->surfaceCapabilities.currentExtent.width;
    float height = (float) vko->surfaceCapabilities.currentExtent.height;
    glm_perspective(GLM_PI_4, width / height, 0.1f, 500.0f, ubo.proj);

    ubo.proj[1][1] *= -1; // flip image upside down bc vulkan and opengl y axes are flipped
    memcpy(vko->cameraUniformBufferMapped[currentImage], &ubo, sizeof(ubo));
}

void synchronizeStreamerAndMeshPoolWithRenderer(World *world, Streamer *streamer, MeshPool *meshPool, vk_context *vko) {
    // invariant: streamer is aligned with mesh pool (not necessarily other way around)

    ChunkPool *chunkPool = &(world->chunkPool);
    ChunkMap *chunkMap = &(world->chunkMap);

    VkCommandBuffer cpyCmd = beginSingleTimeCommands(vko);

    for (int i = 0; i < streamer->size; i++) {
        ChunkHandle c_handle = streamer->activeHandles[i];
        if (c_handle == CHUNK_HANDLE_INVALID) continue;
        Chunk chunk = chunkPool->chunks[c_handle];
        assert(chunk.chunkHandle == c_handle); // why not, just in case
        if (chunk.dirty) {
            meshChunk(c_handle, chunkMap, chunkPool, meshPool, vko, cpyCmd);
        }
    }

    endSingleTimeCommands(vko, cpyCmd);
}

void synchronizePlayerWithChunks(World *world, MeshPool *meshPool, Streamer *streamer) {
    ChunkMap *map = &(world->chunkMap);
    ChunkPool *chunkPool = &(world->chunkPool);
    Camera *cam = &(world->cam);

    int cx = (int) (floor(cam->pos[0] / (float) CHUNK_BLOCK_WIDTH));
    int cy = (int) (floor(cam->pos[1] / (float) CHUNK_BLOCK_WIDTH));

    // need to fix chunk border detection, currently too far away

    // player moved chunks
    // TODO: must time this
    if (cx != cam->chunkPos[0] || cy != cam->chunkPos[1]) {
        cam->chunkPos[0] = cx;
        cam->chunkPos[1] = cy;

        ChunkHandle newHandles[NUM_VISIBLE_CHUNKS]; // newHandles
        int idx = 0;
        for (int y = cy - RENDER_DISTANCE - 1; y <= cy + RENDER_DISTANCE + 1; y++) {
            for (int x = cx - RENDER_DISTANCE - 1; x <= cx + RENDER_DISTANCE + 1; x++) {
                ChunkHandle handle = chunk_map_get(map, x, y);
                if (handle == CHUNK_HANDLE_INVALID) {
                    handle = createChunk(chunkPool, (ivec2) {x, y});
                    chunk_map_put(map, x, y, handle);
                }
                if (y == cy - RENDER_DISTANCE - 1 || y == cy + RENDER_DISTANCE + 1 || x == cx + RENDER_DISTANCE + 1 || x == cx - RENDER_DISTANCE - 1) continue;
                newHandles[idx] = handle;
                idx++;
            }
        }

        int slotsFree[NUM_VISIBLE_CHUNKS];
        int size = 0;
        
        for (int i = 0; i < NUM_VISIBLE_CHUNKS; i++) {
            ChunkHandle oldHandle = streamer->activeHandles[i];

            if (oldHandle == CHUNK_HANDLE_INVALID) {
                slotsFree[size] = i;
                size++;
                continue;
            }

            int oldIsInNew = 0;
            for (int j = 0; j < NUM_VISIBLE_CHUNKS; j++) {
                ChunkHandle newHandle = newHandles[j];
                if (oldHandle == newHandle) {
                    oldIsInNew = 1;
                    newHandles[j] = CHUNK_HANDLE_INVALID;
                    break;
                }
            }

            // this chunk is actually new, free this slot in streamer for new chunks
            if (!oldIsInNew) {
                slotsFree[size] = i;
                size++;
            }
        }

        idx = 0;
        for (int i = 0; i < NUM_VISIBLE_CHUNKS; i++) {
            ChunkHandle handle = newHandles[i];
            if (handle != CHUNK_HANDLE_INVALID) {
                if (!meshPoolIsHandleUsed(*meshPool, handle)) mesh_alloc(meshPool, handle);
                int slot = slotsFree[idx];
                streamer->activeHandles[slot] = handle;
                idx++;
            }
        }
    }
}

// unless implementing multi threading, must synchronize all cpu/gpu data here
void mainLoop(vk_context *vko, Streamer *streamer, World *world, MeshPool *meshPool) {
    uint32_t currentFrame = 0;

    Camera *cam = &world->cam;

    double xpos;
    double ypos;

    glfwGetCursorPos(vko->window, &xpos, &ypos);

    cam->lastX = xpos;
    cam->lastY = ypos;

    mach_timebase_info(&info);

    while (!glfwWindowShouldClose(vko->window)) {
        glfwPollEvents();
        // uint64_t start = mach_absolute_time();
        processInput(vko->window, cam);
        updateCameraUniforms(vko, currentFrame, *cam);
        
        synchronizePlayerWithChunks(world, meshPool, streamer);

        synchronizeStreamerAndMeshPoolWithRenderer(world, streamer, meshPool, vko);
        drawFrame(vko, &currentFrame, *streamer, *meshPool);
        // uint64_t end = mach_absolute_time();
        // float elapsed_ms = (end - start) * info.numer / info.denom * 1e-6;
        // float fps = 1000.0f / elapsed_ms;
        // printf("fps: %f\n", fps);
    }

    vkDeviceWaitIdle(vko->device);
}

void cleanup(vk_context *vko, World *world, Streamer streamer, MeshPool pool) {
    destroyMeshPool(pool, vko);
    destroyWorld(world);
    destroyStreamer(streamer);
    cleanupRenderer(vko);
}

int main() {    
    vk_context vko = {0};

    World world = {0};
    createWorld(&world);

    MeshPool meshPool = {0};
    createMeshPool(&meshPool);

    Streamer streamer = {0};
    createStreamer(&streamer);

    // Player struct; has:
    // Camera camera
    // vec2 pos
    // World allocates space on chunk map to synchronize player with streamer

    ChunkPool *chunkPool = &(world.chunkPool);
    ChunkMap *chunkMap = &(world.chunkMap);
    int num = 0;
    // negative = large cliff (for this height map)
    // positive = small
    for (int y = -RENDER_DISTANCE; y < RENDER_DISTANCE; y++) {
        for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++) {
            ChunkHandle handle = createChunk(chunkPool, (ivec2) {x, y});
            chunk_map_put(chunkMap, x, y, handle);
            streamer.activeHandles[num] = handle; // this should not go past the max
            num++;
        }
    }

    // todo: create player handler

    init_renderer(&vko); // needs to be initialized for chunk meshing
    for (int i = 0; i < num; i++) {
        mesh_alloc(&meshPool, i);
    }

    mainLoop(&vko, &streamer, &world, &meshPool);
    cleanup(&vko, &world, streamer, meshPool);
    
    return 0;
}