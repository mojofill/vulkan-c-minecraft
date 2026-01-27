#include "mesh_pool.h"

void createMeshPool(MeshPool *outMeshPool, uint32_t capacity) {
    outMeshPool->capacity = capacity;
    outMeshPool->meshes = malloc(sizeof(ChunkMesh*) * capacity);
    outMeshPool->handleToSlot = malloc(sizeof(ChunkHandle) * NUM_VISIBLE_CHUNKS);
    outMeshPool->slotsUsed = malloc(sizeof(ChunkHandle) * capacity);
    outMeshPool->count = 0;
    
    for (int i = 0; i < capacity; i++) {
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

void mesh_free(MeshPool *pool, ChunkHandle handle) {
    uint32_t slot = pool->handleToSlot[handle];
    pool->slotsUsed[slot] = 0;
    pool->handleToSlot[handle] = CHUNK_HANDLE_INVALID; // important: must invalidate mesh handle

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

void meshChunk(ChunkHandle handle, World *world, MeshPool *pool, vk_context *vko) {
    int slot = pool->handleToSlot[handle];
    Chunk chunk = world->chunks[handle];
    
    ChunkMesh mesh = {0};
    createChunkMesh(chunk, &mesh, vko);

    printf("finished working on create chunk mesh\n");

    pool->meshes[slot] = mesh;
    chunk.dirty = 0; // after remeshing mark not dirty

    world->chunks[handle] = chunk;
}

void destroyMeshPool(MeshPool meshPool) {
    free(meshPool.meshes);
    free(meshPool.handleToSlot);
    free(meshPool.slotsUsed);
}
