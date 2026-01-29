#include "mesh_pool.h"

void createMeshPool(MeshPool *outMeshPool, uint32_t capacity) {
    outMeshPool->capacity = capacity;
    outMeshPool->meshes = malloc(sizeof(ChunkMesh) * capacity);
    outMeshPool->handleToSlot = malloc(sizeof(ChunkHandle) * NUM_VISIBLE_CHUNKS);
    outMeshPool->slotsUsed = malloc(sizeof(ChunkHandle) * capacity);
    outMeshPool->count = 0;
    
    for (uint32_t i = 0; i < capacity; i++) {
        ChunkMesh mesh = {0};
        outMeshPool->meshes[i] = mesh;
        outMeshPool->slotsUsed[i] = 0;
    }

    for (int i = 0; i < NUM_VISIBLE_CHUNKS; i++) {
        outMeshPool->handleToSlot[i] = CHUNK_HANDLE_INVALID;
    }
}

void mesh_alloc(MeshPool *pool, ChunkHandle handle) {
    // 1. find a free slot
    MeshHandle slot = -1;
    for (uint32_t i = 0; i < pool->capacity; i++) {
        if (pool->slotsUsed[i] == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "failed to find an available slot");
        exit(1);
    }

    // 2. mark slot as used
    pool->slotsUsed[slot] = 1;

    // 3. map mesh handle to slot
    pool->handleToSlot[handle] = slot;

    // 4. initialize mesh (no vertex buffers yet. alloc does not actually upload mesh data, that is for remesh)
    ChunkMesh mesh = {0};
    pool->meshes[slot] = mesh;

    pool->count++;
}

void mesh_free(MeshPool *pool, ChunkHandle handle) {
    uint32_t slot = pool->handleToSlot[handle];
    pool->slotsUsed[slot] = 0;
    pool->handleToSlot[handle] = MESH_SLOT_INVALID; // important: must invalidate mesh handle

    pool->count--;
    if (pool->count < 0) {
        fprintf(stderr, "fatal: count is less than 0\n");
        exit(1);
    }
}

// handle -> slot -> return slot == 0 (free)
int meshPoolIsHandleUsed(MeshPool pool, ChunkHandle handle) {
    if (handle == CHUNK_HANDLE_INVALID) return 0;
    return pool.slotsUsed[pool.handleToSlot[handle]] == 0;
}

static void genChunkMeshVkBuffers(Chunk chunk, ChunkMesh *mesh, vk_context *vko) {
    VkDeviceSize          size = (VkDeviceSize) (sizeof(cube_vertices) * MAX_BLOCKS_PER_CHUNK);
    VkBufferUsageFlags    usage;
    VkMemoryPropertyFlags properties;
    if (mesh->mappedData == NULL) { // need to create transfer and vertex buffers
        usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        createBuffer(vko, size, usage, properties, &mesh->stagingBuffer, &mesh->stagingBufferMemory);

        // map staging buffer to host mapped pointer
        vkMapMemory(vko->device, mesh->stagingBufferMemory, 0, size, 0, (void**) &mesh->mappedData);

        usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        createBuffer(vko, size, usage, properties, &mesh->vertexBuffer, &mesh->vertexBufferMemory);
    }

    // upload block data to mapped pointer
    // for now just add chunk position onto cube vertices

    writeChunkMeshToMappedPointer(chunk, &mesh->mappedData);

    // transfer from staging to vertex buffer
    copyBuffer(vko, mesh->stagingBuffer, mesh->vertexBuffer, size);
}

// creates chunks
// all handles should have already been allocated
void meshChunk(ChunkHandle handle, World *world, MeshPool *pool, vk_context *vko) {
    int slot = pool->handleToSlot[handle];
    Chunk chunk = world->chunks[handle];
    
    if (slot == MESH_SLOT_INVALID) {
        fprintf(stderr, "Failed to allocate handle before calling meshChunk.\n");
        exit(1);
    }
    ChunkMesh *mesh = &pool->meshes[slot];

    genChunkMeshVkBuffers(chunk, mesh, vko);

    chunk.dirty = 0; // after remeshing mark not dirty
    printf("meshing chunk %d\n", chunk.chunkHandle);
    
    // update arrays
    world->chunks[handle] = chunk;
}

void destroyMeshPool(MeshPool meshPool, vk_context *vko) {
    for (int i = 0; i < meshPool.count; i++) {
        vkDestroyBuffer(vko->device, meshPool.meshes[i].vertexBuffer, NULL);
        vkDestroyBuffer(vko->device, meshPool.meshes[i].stagingBuffer, NULL);

        vkFreeMemory(vko->device, meshPool.meshes[i].vertexBufferMemory, NULL);
        vkFreeMemory(vko->device, meshPool.meshes[i].stagingBufferMemory, NULL);
    }
    free(meshPool.meshes);
    free(meshPool.handleToSlot);
    free(meshPool.slotsUsed);
}
