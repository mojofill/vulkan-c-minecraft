#ifndef BUFFER_H
#define BUFFER_H

#include "vk_types.h"
#include "world/chunk.h"

void setVertexBindingDescription(vk_context *vko);
void setVertexAttributeDescriptions(vk_context *vko);
uint32_t findMemoryType(vk_context *vko, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createBuffer(vk_context *vko, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer, VkDeviceMemory *pBufferMemory);
void copyBuffer(vk_context *vko, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void createIndexBuffer(vk_context *vko);
void createVertexBufferContext(vk_context *vko);
VkCommandBuffer beginSingleTimeCommands(vk_context *vko);
void endSingleTimeCommands(vk_context *vko, VkCommandBuffer commandBuffer);
void copyBufferToImage(vk_context *vko, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

#endif