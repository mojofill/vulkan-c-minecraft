#ifndef COMMANDS_H
#define COMMANDS_H

#include "vk_types.h"
#include "mesh/mesh_pool.h"

void createCommandPool(vk_context *vko);
void createCommandBuffers(vk_context *vko);
void resetCommands(vk_context *vko);
void recordCommands(vk_context *vko, uint32_t currentFrame, MeshPool pool);

#endif