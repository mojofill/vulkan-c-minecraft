#include "mesh_pool.h"

void createMeshPool(MeshPool *outMeshPool) {
    outMeshPool->meshes = malloc(sizeof(ChunkMesh) * MAX_LOADED_CHUNKS);
    outMeshPool->handleToSlot = malloc(sizeof(MeshHandle) * MAX_LOADED_CHUNKS);
    outMeshPool->slotsUsed = malloc(sizeof(int) * MAX_LOADED_CHUNKS);
    outMeshPool->count = 0;
    
    for (uint32_t i = 0; i < MAX_LOADED_CHUNKS; i++) {
        ChunkMesh mesh = {0};
        mesh.mappedData = NULL;
        mesh.stagingBuffer = VK_NULL_HANDLE;
        mesh.vertexBuffer = VK_NULL_HANDLE;
        mesh.faceCount = 0;
        outMeshPool->meshes[i] = mesh;
        outMeshPool->slotsUsed[i] = 0;
    }

    for (int i = 0; i < MAX_LOADED_CHUNKS; i++) {
        outMeshPool->handleToSlot[i] = MESH_SLOT_INVALID;
    }
}

void mesh_alloc(MeshPool *pool, ChunkHandle handle) {
    // 1. find a free slot
    MeshHandle slot = -1;
    for (uint32_t i = 0; i < MAX_LOADED_CHUNKS; i++) {
        if (pool->slotsUsed[i] == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        fprintf(stderr, "failed to find an available slot\n");
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
    printf("(IN mesh_free): \n");
    // for (int i = 0; i < 15; i++) printf("%d ", pool->handleToSlot[i]); printf("\n");
}

// handle -> slot -> return slot == 0 (free)
int meshPoolIsHandleUsed(MeshPool pool, ChunkHandle handle) {
    if (handle == CHUNK_HANDLE_INVALID) return 0;
    int slot = pool.handleToSlot[handle];
    return slot != MESH_SLOT_INVALID || pool.slotsUsed[slot] == 0;
}

static void genChunkMeshVkBuffers(Chunk chunk, ChunkMap *chunkMap, ChunkPool *chunkPool, MeshPool *meshPool, vk_context *vko, VkCommandBuffer cpyCmd) {
    size_t possibleSize = (sizeof(cube_vertices) * chunk.num_surface_blocks);
    Vertex *data = malloc(possibleSize);
    int faceCount = 0; // need to update this later
    int res; // this is debug stuff, i should remove this
    int num_faces = writeChunkMeshToMappedPointer(chunk, chunkMap, chunkPool, &data, &faceCount, &res);

    // printf("%d\n", num_faces, CHUNK_BLOCK_WIDTH * CHUNK_BLOCK_WIDTH);

    // 40.975155 being 128 world height, 19352 faces
    // 71.828026 being 256 world height, 21062 faces

    // now approximate how many i gotta add
    // int num_extra = (int) ((21062 - 19352) / (float) NUM_VISIBLE_CHUNKS); // unit: faces
    // num_faces += num_extra;
    // see if it pushes to 71 ms
    // damn... it still goes to 71 ms

    // need to transfer temp pointer to real mapped data
    VkDeviceSize cpySize = num_faces * FACE_SIZE;

    if (cpySize == 0) return; // believe i can just skip this

    // round size up to biggest power of 2 that is just greater than cpySize
    VkDeviceSize capacity = 1;
    while (capacity < cpySize) {
        capacity *= 2;
    }
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;

    int slot = meshPool->handleToSlot[chunk.chunkHandle];
    ChunkMesh *mesh = &(meshPool->meshes[slot]);

    // creating new chunk buffer - need to get rid of vertex meshing next to it
    if (mesh->vertexBuffer == VK_NULL_HANDLE) { // need to create transfer and vertex buffers
        usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        createBuffer(vko, capacity, usage, properties, &mesh->stagingBuffer, &mesh->stagingBufferMemory);
        
        // map staging buffer to host mapped pointer
        // staging buffer should have full capacity
        vkMapMemory(vko->device, mesh->stagingBufferMemory, 0, capacity, 0, (void**) &mesh->mappedData);

        usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        createBuffer(vko, capacity, usage, properties, &mesh->vertexBuffer, &mesh->vertexBufferMemory);

        mesh->capacitySize = capacity;
    }

    memcpy(mesh->mappedData, data, cpySize);
    mesh->faceCount = faceCount;

    VkBufferCopy copyRegion = {0};
    copyRegion.size = cpySize;
    vkCmdCopyBuffer(cpyCmd, mesh->stagingBuffer, mesh->vertexBuffer, 1, &copyRegion);

    free(data);
}

// creates chunks
// all handles should have already been allocated
void meshChunk(ChunkHandle handle, ChunkMap *chunkMap, ChunkPool *chunkPool, MeshPool *meshPool, vk_context *vko, VkCommandBuffer cpyCmd) {
    int slot = meshPool->handleToSlot[handle];
    Chunk *chunk = &(chunkPool->chunks[handle]);
    
    if (slot == MESH_SLOT_INVALID) {
        fprintf(stderr, "Failed to allocate handle before calling meshChunk.\n");
        exit(1);
    }
    
    genChunkMeshVkBuffers(*chunk, chunkMap, chunkPool, meshPool, vko, cpyCmd);

    chunk->dirty = 0; // after remeshing mark not dirty
}

// if block already exists at that spot, will just replace with new block
// this is a blocking command. will fully interact with gpu buffer and update entirely in one call
void meshPutBlock(vk_context *vko, ChunkMap *chunkMap, ChunkPool *chunkPool, MeshPool *meshPool, ChunkHandle chunkHandle, int local_x, int local_y, int local_z, int type) {
    int slot = meshPool->handleToSlot[chunkHandle];
    ChunkMesh *mesh = &meshPool->meshes[slot];

    if (mesh->vertexBuffer == VK_NULL_HANDLE) {
        fprintf(stderr, "mesh vertex buffer is null. this should not happen\n");
        exit(1);
    }

    // first need to see vertex buffer size vs capacity

    Chunk chunk = chunkPool->chunks[chunkHandle];

    VkCommandBuffer cpyCmd = beginSingleTimeCommands(vko);

    // count how many solid blocks there are
    int solids = 0;
    chunk_mesh_foreach(x, y, z) {
        if (z == CHUNK_BLOCK_HEIGHT - 1) continue; // for now, just say the top layer will never be shown
        int at = chunk_mesh_xyz_to_block_index(x,y,z);
        int above = chunk_mesh_xyz_to_block_index(x,y,z+1);
        if (chunk.blocks[at] != AIR && chunk.blocks[above] == AIR) solids++;
    }

    size_t possibleSize = (sizeof(cube_vertices) * solids);
    Vertex *data = malloc(possibleSize);
    int faceCount = 0; // need to update this later
    int res; // this is debug stuff, i should remove this
    int num_faces = writeChunkMeshToMappedPointer(chunk, chunkMap, chunkPool, &data, &faceCount, &res);

    VkDeviceSize cpySize = num_faces * FACE_SIZE;

    // update mesh state
    mesh->faceCount = num_faces;
    if (mesh->capacitySize < cpySize) {
        // double size of mesh capacity
        mesh->capacitySize *= 2;
        
        // reallocate space for in staging and vertex buffer
        vkDestroyBuffer(vko->device, mesh->stagingBuffer, NULL);
        vkDestroyBuffer(vko->device, mesh->vertexBuffer, NULL);
        vkFreeMemory(vko->device, mesh->stagingBufferMemory, NULL);
        vkFreeMemory(vko->device, mesh->vertexBufferMemory, NULL);

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        createBuffer(vko, mesh->capacitySize, usage, properties, &mesh->stagingBuffer, &mesh->stagingBufferMemory);
        
        // map staging buffer to host mapped pointer
        // staging buffer should have full capacity
        vkMapMemory(vko->device, mesh->stagingBufferMemory, 0, mesh->capacitySize, 0, (void**) &mesh->mappedData);

        usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        createBuffer(vko, mesh->capacitySize, usage, properties, &mesh->vertexBuffer, &mesh->vertexBufferMemory);
    }

    // finally, copy data from test pointer to staging buffer pointer
    memcpy(mesh->mappedData, data, cpySize);

    VkBufferCopy copyRegion = {0};
    copyRegion.size = cpySize;
    vkCmdCopyBuffer(cpyCmd, mesh->stagingBuffer, mesh->vertexBuffer, 1, &copyRegion);

    endSingleTimeCommands(vko, cpyCmd);
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
