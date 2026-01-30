#ifndef CHUNK_MESH_H
#define CHUNK_MESH_H

#include "renderer/vk_types.h" // im a little bit scared of this...
#include "renderer/buffer.h"
#include <stdint.h>
#include <cglm/cglm.h>
#include "world/chunk.h"

// for now, each chunk has own buffers. in future need to have chunk buffer allocator, each chunk mesh has subranges and offsets
typedef struct ChunkMesh {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Vertex *mappedData;
} ChunkMesh;

void writeChunkMeshToMappedPointer(Chunk chunk, Vertex **pMappedData);

#endif