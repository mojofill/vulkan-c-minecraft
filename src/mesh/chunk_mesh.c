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
    
    // get rid of annoying warning
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
        glm_vec3_add(local, chunk.pos, worldBlockPos);
    #pragma GCC diagnostic pop

    for (int i = 0; i < 4; i++) {
        Vertex v = cube_vertices[at + i];
        glm_vec3_add(v.pos, worldBlockPos, v.pos);
        (*pMappedData)[*idx] = v;
        (*idx)++;
    }
}

void emitFaceCheck(Chunk chunk, float *localBlockPos, Direction dir, Vertex **pMappedData, int *idx) {
    int x = (int) localBlockPos[0];
    int y = (int) localBlockPos[1];
    int z = (int) localBlockPos[2];

    if      (dir == LEFT)  x--;
    else if (dir == RIGHT) x++;
    else if (dir == UP)    z++;
    else if (dir == DOWN)  z--;
    else if (dir == FRONT) y++;
    else /* (dir == BACK)*/y--;

    // neighboring block out of chunk, emit this face
    if (x < 0 || x >= CHUNK_BLOCK_WIDTH || y < 0 || y >= CHUNK_BLOCK_WIDTH || z < 0 || z >= CHUNK_BLOCK_WIDTH) {
        emitFaceNoCheck(chunk, localBlockPos, dir, pMappedData, idx);
        return;
    }

    // if neighboring block is not air, do not emit this face
    // else emit face

    int i = chunk_mesh_xyz_to_block_index(x, y, z);
    BlockType block = chunk.blocks[i];
    if (block == AIR) {
        printf("emitting face %d\n", *idx);
        emitFaceNoCheck(chunk, localBlockPos, dir, pMappedData, idx);
    }
}

void writeChunkMeshToMappedPointer(Chunk chunk, Vertex **pMappedData) {
    // for x, y in square with width CHUNK_BLOCK_WIDTH do
    //     if block at x-1, y is air then
    //         emitFace(LEFT)
    //     ...repeat for all 6 directions
    
    int idx = 0;
    chunk_mesh_foreach(x, y, z) {
        vec3 blockPos = {(float) x, (float) y, (float) z};

        // vec3 blockPos = {chunk.pos[0] + ((float) x), chunk.pos[1] + ((float) y), (float) z};
        // for (int i = 0; i < CUBE_SIZE; i++) {
        //     Vertex v = cube_vertices[i];
        //     glm_vec3_add(v.pos, blockPos, v.pos);
        //     (*pMappedData)[idx] = v;
        //     idx++;
        // }
        
        for (int dir = 0; dir < 6; dir++) {
            emitFaceCheck(chunk, blockPos, dir, pMappedData, &idx);
        }
    }
}
