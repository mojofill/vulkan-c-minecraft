#ifndef MESH_POOL_H
#define MESH_POOL_H

#include "world/world.h"
#include "chunk_mesh.h"
#include "geometries/vertex.h"
#include <stdlib.h>
#include <stdio.h>

// mesh pool invariants
// 1. handleToSlot: indices = ChunkHandle, value at index = slot index
// 2. arrays: meshes & slots, both size = capacity
//  * (think physical cache registers, physical length = capacity)
//  * initialization: all slots = 0, all meshes = empty mesh
// 3. mesh_alloc: finds first free slot, finds corresponding mesh
//    binds ChunkHandle to slot index, and populate mesh in meshes
// 4. mesh_free: finds slot index corresponding to ChunkHandle via handleToSlot
//  * marks slot as free in slot register, leaves trash data in meshes register,
//    and frees handle/slot binding in handleToSlot (via MESH_INVALID_HANDLE)

typedef uint32_t MeshHandle;
#define MESH_SLOT_INVALID UINT32_MAX
#define MESH_SLOT_OFF 0
#define MESH_SLOT_ON 1

typedef struct MeshPool {
    ChunkMesh *meshes; // size = capacity
    uint32_t capacity;

    MeshHandle *handleToSlot; // size = NUM_VISIBLE_CHUNKS
    uint32_t *slotsUsed; // size = capacity

    uint32_t count;
} MeshPool;

void createMeshPool(MeshPool *outMeshPool);
void destroyMeshPool(MeshPool meshPool, vk_context *vko);
void mesh_alloc(MeshPool *pool, ChunkHandle handle);
void mesh_free(MeshPool *pool, ChunkHandle handle);
int meshPoolIsHandleUsed(MeshPool pool, ChunkHandle handle);
void meshChunk(ChunkHandle handle, ChunkPool *chunkPool, MeshPool *meshPool, vk_context *vko);

#endif