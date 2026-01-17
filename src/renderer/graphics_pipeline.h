#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "vk_types.h"

void createGraphicsPipeline(vk_context *vko);
static VkShaderModule load_shader(VkDevice device, const char* path);

#endif