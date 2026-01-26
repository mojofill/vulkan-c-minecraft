#ifndef MESH_POOL_H
#define MESH_POOL_H

#include "world/chunk.h"
#include "chunk_mesh.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct MeshPool {
    ChunkMesh *meshes; // size = capacity
    uint32_t capacity;

    MeshHandle *handleToSlot; // size = NUM_VISIBLE_CHUNKS
    uint32_t *slotsUsed; // size = capacity

    uint32_t count;
} MeshPool;

void createMeshPool(MeshPool *outMeshPool, uint32_t capacity);
void destroyMeshPool(MeshPool meshPool);
void mesh_alloc(MeshPool *pool, MeshHandle handle);
void mesh_free(MeshPool *pool, MeshHandle handle);

#endif