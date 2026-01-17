#ifndef VERTEX_H
#define VERTEX_H

#define GLM_FORCE_DEPTH_ZERO_TO_ONE // opengl uses depth range -1 -> 1, vulkan uses 0 -> 1
#define GLM_FORCE_RADIANS
#include <cglm/cglm.h>

typedef struct Vertex {
    vec4 pos;
    vec3 color;
    vec2 texCoord;
} Vertex;

#endif