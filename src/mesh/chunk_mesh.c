#include "chunk_mesh.h"

typedef enum Direction {
    LEFT, RIGHT, UP, DOWN, FRONT, BACK
} Direction;

// takes direction and turns them into the correct face index
// will be local, no scaling, such that i is first corner vertex, i+3 is last
// NOTE, THIS HAS TO BE THE *SAME* AS cube_vertices IN VERTEX.H
int dirToIndex(Direction dir) {
    switch (dir) {
        case LEFT:
            return 8;
        case RIGHT:
            return 12;
        case UP:
            return 4;
        case DOWN:
            return 0;
        case FRONT:
            return 20;
        case BACK:
            return 16;
    }
}

void emitFaceNoCheck(Chunk chunk, float *localBlockPos, Direction dir, Vertex **pMappedData, int *idx) {
    int at = dirToIndex(dir);

    vec3 local;
    vec3 worldBlockPos;
    
    glm_vec3_copy(localBlockPos, local);

    vec2 scaledChunkPos;
    scaledChunkPos[0] = (float) chunk.pos[0] * CHUNK_BLOCK_WIDTH;
    scaledChunkPos[1] = (float) chunk.pos[1] * CHUNK_BLOCK_WIDTH;
    
    // get rid of annoying warning
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
        glm_vec3_add(local, scaledChunkPos, worldBlockPos);
    #pragma GCC diagnostic pop

    // 1.0f/16.0f
    float blockUVSize = 0.0625;  // size of one block in UV space
    int col = 3;
    int row = 0;

    float uOffset = col * blockUVSize;
    float vOffset = row * blockUVSize;

    for (int i = 0; i < 4; i++) {
        Vertex v = cube_vertices[at + i];
        v.light = faceLight[dir];
        glm_vec3_add(v.pos, worldBlockPos, v.pos);
        v.texCoord[0] = v.texCoord[0] * blockUVSize + uOffset;
        v.texCoord[1] = v.texCoord[1] * blockUVSize + vOffset;
        (*pMappedData)[*idx] = v;
        (*idx)++;
    }
}

void emitFaceCheck(Chunk chunk, ChunkMap *map, ChunkPool *pool, float *localBlockPos, Direction dir, Vertex **pMappedData, int *idx) {
    int x = (int) localBlockPos[0];
    int y = (int) localBlockPos[1];
    int z = (int) localBlockPos[2];

    if (chunk.blocks[chunk_mesh_xyz_to_block_index(x,y,z)] == AIR) return;

    if      (dir == LEFT)  x--;
    else if (dir == RIGHT) x++;
    else if (dir == UP)    z++;
    else if (dir == DOWN)  z--;
    else if (dir == FRONT) y++;
    else /* (dir == BACK)*/y--;

    // neighboring block out of chunk, check face on neighboring chunk
    if (x < 0 || x >= CHUNK_BLOCK_WIDTH || y < 0 || y >= CHUNK_BLOCK_WIDTH || z < 0 || z >= CHUNK_BLOCK_HEIGHT) {
        if (z < 0) return; // will not be rendering anything below
        if (z >= CHUNK_BLOCK_HEIGHT) {
            emitFaceNoCheck(chunk, localBlockPos, dir, pMappedData, idx); // always render top of chunk
            return;
        }
        
        int cx = chunk.pos[0];
        int cy = chunk.pos[1];

        if (x < 0) {
            x = CHUNK_BLOCK_WIDTH - 1;
            cx--;
        }
        else if (x >= CHUNK_BLOCK_WIDTH) {
            x = 0;
            cx++;
        }
        
        if (y < 0) {
            y = CHUNK_BLOCK_WIDTH - 1;
            cy--;
        }
        else if (y >= CHUNK_BLOCK_WIDTH) {
            y = 0;
            cy++;
        }
        
        ChunkHandle neighborHandle = chunk_map_get(map, cx, cy);

        if (neighborHandle == CHUNK_HANDLE_INVALID) {
            emitFaceNoCheck(chunk, localBlockPos, dir, pMappedData, idx);
            return;
        }

        Chunk neighborChunk = pool->chunks[neighborHandle];

        int neighbor_block_idx = chunk_mesh_xyz_to_block_index(x, y, z);
        BlockType neighbor_block_type = neighborChunk.blocks[neighbor_block_idx];

        if (neighbor_block_type == AIR) emitFaceNoCheck(chunk, localBlockPos, dir, pMappedData, idx);
        else return;
    }

    // if neighboring block is not air, do not emit this face
    // else emit face

    int i = chunk_mesh_xyz_to_block_index(x, y, z);
    BlockType block = chunk.blocks[i];
    if (block == AIR) {
        emitFaceNoCheck(chunk, localBlockPos, dir, pMappedData, idx);
    }
}

void writeChunkMeshToMappedPointer(Chunk chunk, ChunkMap *map, ChunkPool *chunkPool, Vertex **pMappedData) {
    int idx = 0;
    chunk_mesh_foreach(x, y, z) {
        vec3 blockPos = {(float) x, (float) y, (float) z};

        for (int dir = 0; dir < 6; dir++) {
            emitFaceCheck(chunk, map, chunkPool, blockPos, dir, pMappedData, &idx);
        }
    }
}
