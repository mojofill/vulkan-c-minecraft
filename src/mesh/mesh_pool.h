#ifndef MESH_POOL_H
#define MESH_POOL_H

#include "chunk_mesh.h"
#include <stdlib.h>

typedef struct MeshPool {
    ChunkMesh **pool;
    int length;
} MeshPool;

void createMeshPool(MeshPool *outMeshPool, int length);
void destroyMeshPool(MeshPool meshPool);

void remeshChunks(MeshPool pool);

#endif