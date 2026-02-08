#ifndef VERTEX_H
#define VERTEX_H

#define GLM_FORCE_DEPTH_ZERO_TO_ONE // opengl uses depth range -1 -> 1, vulkan uses 0 -> 1
#define GLM_FORCE_RADIANS
#include <cglm/cglm.h>

typedef struct Vertex {
    vec3 pos;
    vec3 color;
    vec2 texCoord;
    uint8_t light;
    uint8_t _pad[3];
} Vertex;

static uint8_t faceLight[6] = {
    120, 
    180, 
    255, // top
    120, 
    100, 
    120  
};

#define FACE_SIZE (4 * sizeof(Vertex))

// number of vertices in cube_vertices[]
#define CUBE_SIZE 24
#define CUBE_SIZE 24
// NOTE: CANNOT CHANGE THIS, IF DO CHANGE ORDER IN CHUNK_MESH.C
static const Vertex cube_vertices[] = {
    // ---- Bottom face (-Z) ----
    {{-0.5f,-0.5f,-0.5f},{1,0,0},{0,0}}, // 0
    {{ 0.5f,-0.5f,-0.5f},{1,0,0},{1,0}}, // 1
    {{ 0.5f, 0.5f,-0.5f},{1,0,0},{1,1}}, // 2
    {{-0.5f, 0.5f,-0.5f},{1,0,0},{0,1}}, // 3

    // ---- Up face (+Z) ----
    {{-0.5f,-0.5f, 0.5f},{0,1,0},{0,0}}, // 4
    {{-0.5f, 0.5f, 0.5f},{0,1,0},{0,1}}, // 5
    {{ 0.5f, 0.5f, 0.5f},{0,1,0},{1,1}}, // 6
    {{ 0.5f,-0.5f, 0.5f},{0,1,0},{1,0}}, // 7

    // ---- Left face (-X) ----
    {{-0.5f,-0.5f,-0.5f},{0,0,1},{1,0}}, // 8
    {{-0.5f, 0.5f,-0.5f},{0,0,1},{1,1}}, // 9
    {{-0.5f, 0.5f, 0.5f},{0,0,1},{0,1}}, //10
    {{-0.5f,-0.5f, 0.5f},{0,0,1},{0,0}}, //11

    // ---- Right face (+X) ----
    {{ 0.5f,-0.5f, 0.5f},{1,1,0},{1,0}}, //12
    {{ 0.5f, 0.5f, 0.5f},{1,1,0},{1,1}}, //13
    {{ 0.5f, 0.5f,-0.5f},{1,1,0},{0,1}}, //14
    {{ 0.5f,-0.5f,-0.5f},{1,1,0},{0,0}}, //15

    // ---- Back face (-Y) ----
    {{-0.5f,-0.5f,-0.5f},{1,0,1},{0,0}}, //16
    {{-0.5f,-0.5f, 0.5f},{1,0,1},{0,1}}, //17
    {{ 0.5f,-0.5f, 0.5f},{1,0,1},{1,1}}, //18
    {{ 0.5f,-0.5f,-0.5f},{1,0,1},{1,0}}, //19

    // ---- Front face (+Y) ----
    {{-0.5f, 0.5f,-0.5f},{0,1,1},{0,0}}, //20
    {{ 0.5f, 0.5f,-0.5f},{0,1,1},{1,0}}, //21
    {{ 0.5f, 0.5f, 0.5f},{0,1,1},{1,1}}, //22
    {{-0.5f, 0.5f, 0.5f},{0,1,1},{0,1}}, //23
};

#define CUBE_INDEX_COUNT 36

static const uint32_t cube_indices[CUBE_INDEX_COUNT] = {
    // back
    0,  1,  2,  2,  3,  0,
    // front
    4,  5,  6,  6,  7,  4,
    // left
    8,  9, 10, 10, 11,  8,
    // right
    12, 13, 14, 14, 15, 12,
    // bottom
    16, 17, 18, 18, 19, 16,
    // top
    20, 21, 22, 22, 23, 20
};

#define INDICES_PER_CUBE 36

// 1.0f/16.0f
// size of one block in UV space
#define blockUVSize 0.0625

#endif