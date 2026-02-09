#ifndef CHUNK_MESH_H
#define CHUNK_MESH_H

#include "renderer/vk_types.h"
#include "renderer/buffer.h"
#include <stdint.h>
#include <cglm/cglm.h>
#include "world/chunk_map.h"
#include "world/chunk_pool.h"
#include "world/block.h"

// for now, each chunk has own buffers. in future need to have chunk buffer allocator, each chunk mesh has subranges and offsets
typedef struct ChunkMesh {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Vertex *mappedData;
    uint32_t faceCount;
} ChunkMesh;

uint32_t writeChunkMeshToMappedPointer(Chunk chunk, ChunkMap *map, ChunkPool *chunkPool, Vertex **pMappedData, uint32_t *pMeshFaceCount, int *res);
void emitFaceNoCheck(Chunk chunk, float *localBlockPos, Direction dir, Vertex **pMappedData, int *idx);
int emitFaceCheck(Chunk chunk, ChunkMap *map, ChunkPool *pool, float *localBlockPos, Direction dir, Vertex **pMappedData, int *idx, int *res);

#endif