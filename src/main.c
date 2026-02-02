#include "renderer/renderer.h"
#include "mesh/mesh_pool.h"
#include "world/world.h"
#include "streamer/streamer.h"
#include "mach/mach_time.h"
#include <math.h>

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

    float x = cam.pos[0];
    float y = cam.pos[1];
    float z = cam.pos[2];
    // printf("camera pos: %f, %f, %f\n", cam.pos[0], cam.pos[1], cam.pos[2]); // try to guess which chunk im in

    glm_lookat(cam.pos, forward, (vec3){0.0f, 0.0f, 1.0f}, ubo.view);

    float width = (float) vko->surfaceCapabilities.currentExtent.width;
    float height = (float) vko->surfaceCapabilities.currentExtent.height;
    glm_perspective(GLM_PI_4, width / height, 0.1f, 100.0f, ubo.proj);

    ubo.proj[1][1] *= -1; // flip image upside down bc vulkan and opengl y axes are flipped
    memcpy(vko->cameraUniformBufferMapped[currentImage], &ubo, sizeof(ubo));
}

void synchronizeStreamerAndMeshPoolWithRenderer(World *world, Streamer *streamer, MeshPool *meshPool, vk_context *vko) {
    // invariant: streamer is aligned with mesh pool (not necessarily other way around)

    // streamer + mesh pool <-> renderer synchronization:
    // a. for each mesh handle in streamer
    //    if chunk is dirty then // (find chunk from chunk handle via world.chunks)
    //        remesh chunk (connects to mapped memory pointers)
    // b. renderer will see all chunk data, including those remeshed, and will render those
    //    for each mesh in mesh pool do
    //        vk draw chunk vertex buffer + global index buffer (same index buffer for each chunk)

    ChunkPool *chunkPool = &(world->chunkPool);

    for (int i = 0; i < streamer->size; i++) {
        ChunkHandle c_handle = streamer->activeHandles[i];
        if (c_handle == CHUNK_HANDLE_INVALID) continue;
        Chunk chunk = chunkPool->chunks[c_handle];
        assert(chunk.chunkHandle == c_handle); // why not, just in case
        if (chunk.dirty) {
            meshChunk(c_handle, chunkPool, meshPool, vko);
        }
    }
}

void synchronizePlayerWithChunks(World *world, MeshPool *meshPool, Streamer *streamer) {
    ChunkMap *map = &(world->chunkMap);
    ChunkPool *chunkPool = &(world->chunkPool);
    Camera *cam = &(world->cam);

    int cx = (int) (floor(cam->pos[0] / (float) CHUNK_BLOCK_WIDTH));
    int cy = (int) (floor(cam->pos[1] / (float) CHUNK_BLOCK_WIDTH));

    // need to fix chunk border detection, currently too far away

    // player moved chunks
    if (cx != cam->chunkPos[0] || cy != cam->chunkPos[1]) {
        cam->chunkPos[0] = cx;
        cam->chunkPos[1] = cy;

        ChunkHandle newHandles[NUM_VISIBLE_CHUNKS]; // newHandles
        int idx = 0;
        for (int y = cy - RENDER_DISTANCE; y <= cy + RENDER_DISTANCE; y++) {
            for (int x = cx - RENDER_DISTANCE; x <= cx + RENDER_DISTANCE; x++) {
                ChunkHandle handle = chunk_map_get(map, x, y);
                if (handle == CHUNK_HANDLE_INVALID) {
                    handle = createChunk(chunkPool, (ivec2) {x, y});
                    chunk_map_put(map, x, y, handle);
                }
                newHandles[idx] = handle;
                chunkPool->chunks[handle].dirty = 1; // need to set new chunks to dirty
                idx++;
            }
        }

        // newHandles = new handles this frame that need to be rendered
        // streamer.activeHandles = old handles that need to be updated

        // for each oldHandle in streamer.activeHandles do
        //     for each newHandle in newHandles do
        //         if (oldHandle == newHandle) then
        //             oldHandle is in newHandles
        //             newHandles[new handle index] = CHUNK_HANDLE_INVALID; 
        //             can get rid of this one in newHandles because its actually old
        //             break
        //     if oldHandle is not in newHandles then
        //         slotsFree.push(oldHandle slot in activeHandles)
        //
        // for each handle in newHandles do
        //     if handle is not invalid then
        //         mesh_alloc(pool, handle)
        //         streamer.activeHandles[next available slot] = handle

        int slotsFree[NUM_VISIBLE_CHUNKS];
        int size = 0;
        
        for (int i = 0; i < NUM_VISIBLE_CHUNKS; i++) {
            ChunkHandle oldHandle = streamer->activeHandles[i];
            int oldIsInNew = 0;
            for (int j = 0; j < NUM_VISIBLE_CHUNKS; j++) {
                ChunkHandle newHandle = newHandles[j];
                if (oldHandle == newHandle) {
                    oldIsInNew = 1;
                    newHandles[j] = CHUNK_HANDLE_INVALID;
                    break;
                }
            }

            if (!oldIsInNew) {
                slotsFree[size] = i;
                size++;
            }
        }

        idx = 0;
        for (int i = 0; i < NUM_VISIBLE_CHUNKS; i++) {
            ChunkHandle handle = newHandles[i];
            if (handle != CHUNK_HANDLE_INVALID) {
                mesh_alloc(meshPool, handle);
                int slot = slotsFree[idx];
                streamer->activeHandles[slot] = handle;
                idx++;
            }
        }
    }
}

// unless implementing multi threading, must synchronize all cpu/gpu data here
void mainLoop(vk_context *vko, Streamer *streamer, World *world, MeshPool *pool) {
    uint32_t currentFrame = 0;

    mach_timebase_info_data_t info;
    mach_timebase_info(&info);

    while (!glfwWindowShouldClose(vko->window)) {
        // uint64_t start_time = mach_absolute_time();
        glfwPollEvents();
        processInput(vko->window, &world->cam);
        updateCameraUniforms(vko, currentFrame, world->cam);

        // invariant: streamer is aligned with mesh pool (must keep this in mind)
        //     ie any changes made to streamer must reflect mesh pool
        //     (not *necessarily* the other way around)

        // 1. synchronize player with streamer + mesh pool
        // 2. synchronize streamer + mesh pool with renderer
        
        // player <-> streamer synchronization: 
        // a. if player moved chunks, move render borders by removing/adding to streamer + mesh_free/mesh_alloc meshes
        // b. for each chunk, if chunk dirty, remesh data with updated chunk data

        // check synchronizeStreamerAndMeshPoolWithRenderer for description of streamer + mesh pool synchronization with renderer

        // these might need to put in a separate thread
        
        // 1. synchronizer player with streamer
        synchronizePlayerWithChunks(world, pool, streamer);

        // 2. synchronize streamer + mesh pool with renderer
        synchronizeStreamerAndMeshPoolWithRenderer(world, streamer, pool, vko);
        
        drawFrame(vko, &currentFrame, *streamer, *pool);
        // uint64_t end_time = mach_absolute_time();
        // uint64_t duration = end_time - start_time;
        // uint64_t duration_ns = duration * info.numer / info.denom;
        // double fps = 1000000000 / (double) duration_ns;
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
    for (int y = -RENDER_DISTANCE; y <= RENDER_DISTANCE; y++) {
        for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
            ChunkHandle handle = createChunk(chunkPool, (ivec2) {x, y});
            chunk_map_put(chunkMap, x, y, handle);
            streamer.activeHandles[handle] = handle; // this should not go past the max
            num++;
        }
    }

    // todo: create player handler

    init_renderer(&vko); // needs to be initialized for chunk meshing
    for (int i = 0; i < num; i++) {
        mesh_alloc(&meshPool, i);
        meshChunk(i, &(world.chunkPool), &meshPool, &vko);
    }

    mainLoop(&vko, &streamer, &world, &meshPool);
    cleanup(&vko, &world, streamer, meshPool);
    
    return 0;
}