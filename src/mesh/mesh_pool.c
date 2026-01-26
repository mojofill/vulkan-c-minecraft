#include "mesh_pool.h"

void createMeshPool(MeshPool *outMeshPool, uint32_t capacity) {
    outMeshPool->capacity = capacity;
    outMeshPool->meshes = malloc(sizeof(ChunkMesh*) * capacity);
    outMeshPool->handleToSlot = malloc(sizeof(MeshHandle) * NUM_VISIBLE_CHUNKS);
    outMeshPool->slotsUsed = malloc(sizeof(MeshHandle) * capacity);
    outMeshPool->count = 0;
    
    for (int i = 0; i < capacity; i++) {
        ChunkMesh mesh = {0};
        outMeshPool->meshes[i] = mesh;
        outMeshPool->slotsUsed[i] = 0;
    }

    for (int i = 0; i < NUM_VISIBLE_CHUNKS; i++) {
        outMeshPool->handleToSlot[i] = MESH_HANDLE_INVALID;
    }
}

void mesh_alloc(MeshPool *pool, MeshHandle handle) {
    // 1. find a free slot
    uint32_t slot = -1;
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

void mesh_free(MeshPool *pool, MeshHandle handle) {
    uint32_t slot = pool->handleToSlot[handle];
    pool->slotsUsed[slot] = 0;
    pool->handleToSlot[handle] = MESH_HANDLE_INVALID; // important: must invalidate mesh handle

    pool->count--;
    if (pool->count < 0) {
        fprintf(stderr, "fatal: count is less than 0\n");
        exit(1);
    }
}

void destroyMeshPool(MeshPool meshPool) {
    free(meshPool.meshes);
    free(meshPool.handleToSlot);
    free(meshPool.slotsUsed);
}
