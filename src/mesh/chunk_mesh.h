#ifndef CHUNK_MESH_H
#define CHUNK_MESH_H

#include <vulkan/vulkan.h>
#include <stdint.h>
#include <cglm/cglm.h>

typedef uint32_t MeshHandle;
#define MESH_HANDLE_INVALID UINT32_MAX

typedef struct ChunkMesh {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
} ChunkMesh;

void remesh(ChunkMesh *mesh, vec3 pos);

#endif