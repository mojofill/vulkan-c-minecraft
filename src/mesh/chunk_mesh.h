#ifndef CHUNK_MESH_H
#define CHUNK_MESH_H

#include "renderer/vk_types.h" // im a little bit scared of this...
#include "renderer/buffer.h"
#include <stdint.h>
#include <cglm/cglm.h>
#include "world/chunk.h"

#define chunk_mesh_foreach(x, y, z) \
    for (int z = 0; z < CHUNK_BLOCK_HEIGHT; z++) \
        for (int y = 0; y < CHUNK_BLOCK_WIDTH; y++) \
            for (int x = 0; x < CHUNK_BLOCK_WIDTH; x++)

#define chunk_mesh_xyz_to_block_index(x, y, z) \
    ((z) * CHUNK_BLOCK_WIDTH * CHUNK_BLOCK_HEIGHT + \
     (y) * CHUNK_BLOCK_WIDTH + \
     (x))

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