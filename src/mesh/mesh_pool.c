#include "mesh_pool.h"

void createMeshPool(MeshPool *outMeshPool, int length) {
    outMeshPool->length = length;
    outMeshPool->pool = malloc(sizeof(ChunkMesh*) * length);
    for (int i = 0; i < length; i++) {
        ChunkMesh *mesh = malloc(sizeof(ChunkMesh));
        outMeshPool->pool[i] = mesh;
    }
}

void remeshChunks(MeshPool pool) {
    
}

void destroyMeshPool(MeshPool meshPool) {
    for (int i = 0; i < meshPool.length; i++) {
        free(meshPool.pool[i]);
    }
}
